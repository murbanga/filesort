#include <stdio.h>
#include <string>

using namespace std;

template <char a, char ...Chars> class FormatContents;

template <char a, char ...Chars> class Formatter;

#if 0
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
#endif

template <char ...Chars>
class Formatter<'\0', Chars...>
{
public:
    static string text() { return ""; }
};

template <char a, char ...Chars>
class Formatter :public Formatter<Chars...>
{
public:
    template <class ...Types>
    static string text(Types ...vals)
    {
        string s = Formatter<Chars...>::text<Types...>(vals...);
        return string({ a }) + s; 
    }

    static string text()
    {
        string s = Formatter<Chars...>::text();
        return string({ a }) + s;
    }
};

template <char ...Chars>
class Formatter<'%', Chars...> :public FormatContents<Chars...>
{
public:
    template <class T, class ...Types>
    static string text(T val, Types ...vals) { return formatted("",val) + FormatContents<Chars...>::text<Types...>(vals...); }
};

#define FORMAT_CONTENTS(sym)    \
    template <char ...Chars>  class FormatContents<sym, Chars...> :public FormatContents<Chars...>   \
    {\
    public:\
        template <class T>\
        static string formatted(const string &format, T value) { return FormatContents<Chars...>::formatted(format + sym, value); }\
    };

FORMAT_CONTENTS('-');
FORMAT_CONTENTS('+');
FORMAT_CONTENTS(' ');
FORMAT_CONTENTS('.');
FORMAT_CONTENTS('1');
FORMAT_CONTENTS('2');
FORMAT_CONTENTS('3');
FORMAT_CONTENTS('4');
FORMAT_CONTENTS('5');
FORMAT_CONTENTS('6');
FORMAT_CONTENTS('7');
FORMAT_CONTENTS('8');
FORMAT_CONTENTS('9');

#define TERMINAL_FORMATTER(sym, type)   \
template <char ...Chars> class FormatContents<sym, Chars...> :public Formatter<Chars...>\
{\
public:\
    static string formatted(const string &format, type value) { char buf[256]; _snprintf(buf, sizeof(buf), (format+sym).c_str(), value); return buf; }\
};

TERMINAL_FORMATTER('f', float);
TERMINAL_FORMATTER('d', int);
TERMINAL_FORMATTER('p', void *);

template <char ...Chars> class FormatContents<'s', Chars...> :public Formatter<Chars...>
{
public:\
    static string formatted(const string &format, const string &value) { char buf[256]; _snprintf(buf, sizeof(buf), (format+'s').c_str(), static_cast<string>(value).c_str()); return buf; }
};

/*template <char ...Chars>
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
{};*/

#define LETTER(str,pos) (pos < sizeof(str) ? str[pos] : 0)

#define PARSE(str)  \
	LETTER(str,0),LETTER(str,1),LETTER(str,2),LETTER(str,3),LETTER(str,4),\
	LETTER(str,5),LETTER(str,6),LETTER(str,7),LETTER(str,8),LETTER(str,9),\
	LETTER(str,10),LETTER(str,11),LETTER(str,12),LETTER(str,13),LETTER(str,14),\
	LETTER(str,15),LETTER(str,16),LETTER(str,17),LETTER(str,18),LETTER(str,19),\
	LETTER(str,20),LETTER(str,21),LETTER(str,22),LETTER(str,23),LETTER(str,24),\
	LETTER(str,25),LETTER(str,26),LETTER(str,27),LETTER(str,28),LETTER(str,29)
	

#define custom_printf(format, ...)  _custom_printf(format, Formatter<PARSE(format)>(), __VA_ARGS__)

template <class Formatter, class ...Args>
void _custom_printf(const char *format, Formatter checker, const Args&... args)
{
    string s = Formatter::text<Args...>(args...);
    puts(s.c_str());
}

int main(int argc, char **argv)
{
    string s = "govno";
    int i = 13;
    double epsilon = 0.001;

    custom_printf("%d %s %f\n", i, s, 3.14f);
    custom_printf("pointer points to %p address", &i);
    custom_printf("invalid epsilon value %.4f", epsilon);
    //custom_printf("custom message \"%s\"\n", "message");
    //custom_printf("%d %s %f", 1, 0, 2); // compilation error
    return 0;
}
