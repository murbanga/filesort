#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>

using namespace std;

template <typename T>
size_t check_sorted(FILE *f)
{
	fseek(f, 0, SEEK_SET);
	T prev = 0;
	size_t i = 0;

	while (!feof(f)){
		T v;
		size_t read = fread(&v, sizeof(v), 1, f);
		if (v < prev)
			return i;
		prev = v;
		i++;
	}
	return -1;
}

template <typename T>
void partial_sort(FILE *f, FILE *tmp, size_t chunk_size, size_t chunks, int nthreads)
{
	vector < vector < T >> buf(nthreads);
	printf("sorting %d chunks\n", chunks);
	for (size_t i = 0; i < chunks; i += nthreads){
		thread last;
		for (int j = 0; j < nthreads && j + i < chunks; ++j){
			buf[j].resize(chunk_size);
			fseek(f, (i + j)*chunk_size*sizeof(T), SEEK_SET);
			size_t read = fread(buf[j].data(), sizeof(T), chunk_size, f);

			thread t([&tmp](vector<T> &buf, int size, thread &prev)
			{
				sort(buf.begin(), buf.begin() + size);

				// wait until previous thread writes down it's data
				if (prev.joinable())prev.join();
				fwrite(buf.data(), sizeof(T), size, tmp);
			}, buf[j], read, move(last));
			last = move(t);
		}
		last.join();
	}
}

// this merge() differs from STL one, it allows to mergin into destination buffer of smaller size
template<typename readIt, typename writeIt>
void merge(readIt &begin0, readIt end0, readIt &begin1, readIt end1, writeIt &begin, writeIt end)
{
	while (begin != end && begin0 != end0 && begin1 != end1){
		while (begin != end && begin0 != end0 && *begin0 <  *begin1)
			*begin++ = *begin0++;
		while (begin != end && begin1 != end1 && *begin0 >= *begin1)
			*begin++ = *begin1++;
	}
	while (begin != end && begin0 != end0)
		*begin++ = *begin0++;
	while (begin != end && begin1 != end1)
		*begin++ = *begin1++;
}

template<typename T>
void shift(T *write, const T *begin, const T *end)
{
	if (write == begin)return;

	while (begin != end)*write++ = *begin++;
}

template <typename T>
void partial_merge(FILE *in, FILE *out, size_t chunk_size, size_t file_size, size_t merge_size)
{
	T *mergebuf = new T[merge_size];
	T *partbuf0 = new T[merge_size];
	T *partbuf1 = new T[merge_size];

	size_t total_written = 0;

	assert(chunk_size < file_size);
	assert(merge_size <= chunk_size);

	fseek(out, 0, SEEK_SET);

	for (size_t i = 0; i < file_size; i += 2 * chunk_size){
		// offset in chunk
		size_t offset0 = 0;
		size_t offset1 = 0;

		// size of data stored in partbufs
		size_t bufsize0 = 0;
		size_t bufsize1 = 0;

		// size of data to read from file
		size_t size0 = min(merge_size, file_size - i);
		size_t size1 = min(merge_size, file_size - i - chunk_size);

		do{
			if (size0 > 0){
				fseek(in, (i + offset0)*sizeof(T), SEEK_SET);
				size_t read = fread(partbuf0 + bufsize0, sizeof(T), size0, in);
				bufsize0 += read;
				offset0 += read;
			}
			
			if (size1 > 0){
				fseek(in, (i + chunk_size + offset1)*sizeof(T), SEEK_SET);
				size_t read = fread(partbuf1 + bufsize1, sizeof(T), size1, in);
				bufsize1 += read;
				offset1 += read;
			}

			const T *start0 = partbuf0;
			const T *start1 = partbuf1;
			T *start = mergebuf;

			merge(start0, start0 + bufsize0, start1, start1 + bufsize1, start, mergebuf + merge_size);
			
			fwrite(mergebuf, sizeof(T), start - mergebuf, out);
			total_written += start - mergebuf;

			shift(partbuf0, start0, partbuf0 + bufsize0);
			shift(partbuf1, start1, partbuf1 + bufsize1);

			bufsize0 -= size_t(start0 - partbuf0);
			bufsize1 -= size_t(start1 - partbuf1);

			size0 = min(merge_size - bufsize0, chunk_size - offset0);
			size1 = min(merge_size - bufsize1, chunk_size - offset1);
		} while (bufsize0 + bufsize1 > 0);
	}

//	assert(total_written == file_size);

	delete[] mergebuf;
	delete[] partbuf0;
	delete[] partbuf1;
}

int main(int argc, char **argv)
{
	string filename;
	string output;
	size_t chunk_size = 0;
	bool verify = false;
	int nthreads = 4;

	for (int i = 1; i < argc; ++i){
		if (string(argv[i]) == "-i")filename = argv[++i];
		if (string(argv[i]) == "-o")output = argv[++i];
		else if (string(argv[i]) == "-c")chunk_size = (atoi(argv[++i]) << 10) / sizeof(uint32_t);
		else if (string(argv[i]) == "-v")verify = true;
		else if (string(argv[i]) == "-t")nthreads = atoi(argv[++i]);
	}

	FILE *f = fopen(filename.c_str(), "rb+");
	if (!f){
		printf("fail to open file %s\n", filename.c_str());
		return -1;
	}

	FILE *tmp = tmpfile();
	if (!tmp){
		printf("fail to open temp file %s\n");
		return -2;
	}

	FILE *out = fopen(output.c_str(), "wb+");
	if (!out){
		printf("fail to open file %s\n", output.c_str());
		return -1;
	}

	fseek(f, 0, SEEK_END);
	size_t filesize = ftell(f) / sizeof(uint32_t);
	size_t n = (filesize + chunk_size - 1) / chunk_size;

	partial_sort<uint32_t>(f, tmp, chunk_size, n, nthreads);

	// merge stage
	size_t total_written = 0;
	size_t merge_part = chunk_size;

	printf("merging by %u elemets at a time\n", merge_part);

	size_t merge_chunk = chunk_size;
	FILE *interim[2] = { tmp, out };
	int idx = 0;

	do{
		printf("merging chunks of %u\n", merge_chunk);
		partial_merge<uint32_t>(interim[idx], interim[idx ^ 1], merge_chunk, filesize, merge_part);
		merge_chunk += merge_chunk;
		idx ^= 1;
	} while (merge_chunk < filesize);
	// this means last merge has been done into output file
	assert(idx == 1);
	
	fclose(out);
	fclose(tmp);
	fclose(f);

	if (verify){
		FILE *f = fopen(output.c_str(), "rb");
		size_t i = check_sorted<uint32_t>(f);
		fclose(f);
		if (i == UINT32_MAX){
			printf("pass\n");
		}
		else{
			printf("check failed at %u element\n", i);
		}
	}

	return 0;
}