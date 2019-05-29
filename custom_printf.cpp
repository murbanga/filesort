#include <stdio.h>
#include <string>

using namespace std;

template <char a, char ...Chars> class Formatter;

template <class T, char ...Chars>
class Validator :public Formatter<Chars...>
{
public:
    template <class Type, class ...Types>
    static void valid(Type a, Types... others);

    template <class Type>
    static void valid(Type a);
};

template <class T, char ...Chars>
template <class Type, class ...Types>
void Validator<T, Chars...>::valid(Type a, Types... others)
{
    static_assert(is_convertible<T, Type>::value, "conversion of argument failed");
    Formatter<Chars...>::valid(others...);
}

template <class T, char ...Chars>
template <class Type>
void Validator<T, Chars...>::valid(Type a)
{
    static_cast<T>(a);
}

template <char a, char ...Chars>
class Formatter :public Formatter<Chars...>
{
};

template <char ...Chars>
class Formatter<'%', 'd', Chars...> : public Validator<int, Chars...>
{};

template <char ...Chars>
class Formatter<'%', 's', Chars...> : public Validator<string, Chars...>
{};

template <char ...Chars>
class Formatter<'%', 'f', Chars...> : public Validator<float, Chars...>
{};

template <char ...Chars>
class Formatter<'%', 'p', Chars...> :public Validator<void*, Chars...>
{};

template <char ...Chars>
class Formatter<'\0', Chars...>
{
public:
    static void valid() {}
};

#define LETTER(str,pos) (pos < sizeof(str) ? str[pos] : 0)

#define PARSE(str)  Formatter<\
	LETTER(str,0),LETTER(str,1),LETTER(str,2),LETTER(str,3),LETTER(str,4),\
	LETTER(str,5),LETTER(str,6),LETTER(str,7),LETTER(str,8),LETTER(str,9),\
	LETTER(str,10),LETTER(str,11),LETTER(str,12),LETTER(str,13),LETTER(str,14),\
	LETTER(str,15),LETTER(str,16),LETTER(str,17),LETTER(str,18),LETTER(str,19),\
	LETTER(str,20),LETTER(str,21),LETTER(str,22),LETTER(str,23),LETTER(str,24),\
	LETTER(str,25),LETTER(str,26),LETTER(str,27),LETTER(str,28),LETTER(str,29)\
	>()

#define CUSTOM_PRINTF(format, ...)  custom_printf(format, PARSE(format), __VA_ARGS__)

template <class Formatter, class ...Args>
void custom_printf(const char *format, Formatter checker, const Args&... args)
{
    Formatter::valid<Args...>(args...);


}

int main(int argc, char **argv)
{
    string s = "govno";
    int i = 13;

    CUSTOM_PRINTF("%d %s %f\n", i, s, 3.14f);
    CUSTOM_PRINTF("pointer points to %p address", &i);
    //CUSTOM_PRINTF("%d %s %f", 1, 0, 2); // compilation error
    return 0;
}
