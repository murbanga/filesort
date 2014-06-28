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
		else if (string(argv[i]) == "-c")chunk_size = (atoi(argv[++i]) << 20) / sizeof(uint32_t);
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

	FILE *out = fopen(output.c_str(), "wb");
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
	size_t merge_part = chunk_size / n;

	printf("merging by %d chunks at a time\n", merge_part);

	vector<uint32_t> mergedbuf[2];
	vector<uint32_t> part(merge_part);

	mergedbuf[0].resize(chunk_size);
	mergedbuf[1].resize(chunk_size);

	for (size_t j = 0; j < chunk_size; j += merge_part){
		size_t merged_size = 0;
		int idx = 0;
		size_t read_size = min(merge_part, chunk_size - j);

		for (size_t i = 0; i < n; ++i, idx ^= 1){
			int64_t seek_pos = i*chunk_size + j;
			if (seek_pos >= filesize)break;

			read_size = min(read_size, size_t(filesize - seek_pos));
			
			_fseeki64(tmp, (i*chunk_size + j)*sizeof(uint32_t), SEEK_SET);
			size_t read = fread(part.data(), sizeof(uint32_t), read_size, tmp);
			
			merge(part.begin(), part.begin() + read,
				mergedbuf[idx].begin(), mergedbuf[idx].begin() + merged_size, mergedbuf[idx ^ 1].begin());
			merged_size += read;
		}
		fwrite(mergedbuf[idx].data(), sizeof(uint32_t), merged_size, out);
		total_written += merged_size;
	}
	assert(total_written == filesize);

	fclose(out);
	fclose(tmp);
	fclose(f);

	if (verify){
		FILE *f = fopen(output.c_str(), "rb");
		size_t i = check_sorted<uint32_t>(f);
		fclose(f);
		if (i < 0){
			printf("pass\n");
		}
		else{
			printf("check failed at %u element\n", i);
		}
	}

	return 0;
}