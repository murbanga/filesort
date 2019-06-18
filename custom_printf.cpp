#include <stdio.h>
#include <string>
#include <vector>
#include <list>

using namespace std;

template <class T>
string _format(const string &format, T value)
{
    char buf[256];
    snprintf(buf, sizeof(buf), format.c_str(), value);
    return buf;
}

template <char a, char ...Chars> class FormatSpecifier;

template <char a, char ...Chars>
class Formatter : public Formatter<Chars...>
{
public:
    template <class ...Types>
    string text(Types ...vals)
    {
        return string({ a }) + Formatter<Chars...>::text(vals...);
    }
};

template <char ...Chars>
class Formatter<'\0', Chars...>
{
public:
    string text()
    {
        return "";
    }
};

template <char ...Chars>
class Formatter<'%', '%', Chars...> :public Formatter<Chars...>
{
public:
    template <class ...Types>
    string text(Types ...vals)
    {
        return "%" + Formatter<Chars...>::text(vals...);
    }
};

template <char ...Chars>
class Formatter<'%', Chars...> :public FormatSpecifier<Chars...>
{
public:
    template <class T, class ...Types>
    string text(T val, Types ...vals)
    {
        string a = FormatSpecifier<Chars...>::formatted("%", val);
        string b = FormatSpecifier<Chars...>::text(vals...);
        return a + b;
    }
};

template <char ...Chars> class FormatSpecifier<'s', Chars...> :public Formatter<Chars...>
{
public:
    string formatted(const string &format, const string &value)
    {
        char buf[256];
        snprintf(buf, sizeof(buf), (format + 's').c_str(), value.c_str());
        return buf;
    }
};

template <char ...Chars> class FormatSpecifier<'l', 's', Chars...> :public Formatter<Chars...>
{
public:
    string formatted(const string &format, const wstring &value)
    {
        char buf[256];
        snprintf(buf, sizeof(buf), (format + "ls").c_str(), value.c_str());
        return buf;
    }
};

template <char ...Chars> class FormatSpecifier<'v', Chars...> : public FormatSpecifier<Chars...>
{
public:
    template <class Container>
    string formatted(const string &format, const Container &vec)
    {
        string s = "[";
        for(auto &v : vec)
        {
            s.append(FormatSpecifier<Chars...>::formatted(format, v)).append(",");
        }
        s.erase(s.end() - 1);
        s.append("]");
        return s;
    }
};

#define FORMAT_SPECIFIER(sym)    \
    template <char ...Chars>  class FormatSpecifier<sym, Chars...> :public FormatSpecifier<Chars...>   \
    {\
    public:\
        template <class T>\
        string formatted(const string &format, T value) { return FormatSpecifier<Chars...>::formatted(format + sym, value); }\
    };

FORMAT_SPECIFIER('-');
FORMAT_SPECIFIER('+');
FORMAT_SPECIFIER(' ');
FORMAT_SPECIFIER('.');
FORMAT_SPECIFIER('0');
FORMAT_SPECIFIER('1');
FORMAT_SPECIFIER('2');
FORMAT_SPECIFIER('3');
FORMAT_SPECIFIER('4');
FORMAT_SPECIFIER('5');
FORMAT_SPECIFIER('6');
FORMAT_SPECIFIER('7');
FORMAT_SPECIFIER('8');
FORMAT_SPECIFIER('9');

#define FORMAT_TERMINATOR(sym, type)   \
template <char ...Chars> class FormatSpecifier<sym, Chars...> :public Formatter<Chars...>\
{\
public:\
    string formatted(const string &format, type value){ return _format<type>(format + sym, value);   }\
};

#define FORMAT_TERMINATOR1(a, b, type)   \
template <char ...Chars> class FormatSpecifier<a, b, Chars...> :public Formatter<Chars...>\
{\
public:\
    string formatted(const string &format, type value){ return _format<type>(format + string({a,b}), value);   }\
};

#define FORMAT_TERMINATOR2(a, b, c, type)   \
template <char ...Chars> class FormatSpecifier<a, b, c, Chars...> :public Formatter<Chars...>\
{\
public:\
    string formatted(const string &format, type value){ return _format<type>(format + string({a,b,c}), value);   }\
};

FORMAT_TERMINATOR('f', double);
FORMAT_TERMINATOR('e', double);
FORMAT_TERMINATOR('g', double);

FORMAT_TERMINATOR1('L', 'f', long double);
FORMAT_TERMINATOR1('L', 'e', long double);
FORMAT_TERMINATOR1('L', 'g', long double);

FORMAT_TERMINATOR('p', void *);

FORMAT_TERMINATOR('c', int);
FORMAT_TERMINATOR1('l', 'c', wint_t);

FORMAT_TERMINATOR('d', int);
FORMAT_TERMINATOR('i', int);

FORMAT_TERMINATOR('o', unsigned int);
FORMAT_TERMINATOR('x', unsigned int);
FORMAT_TERMINATOR('X', unsigned int);
FORMAT_TERMINATOR('u', unsigned int);

FORMAT_TERMINATOR1('h', 'd', short);
FORMAT_TERMINATOR1('h', 'i', short);

FORMAT_TERMINATOR1('h', 'o', unsigned short);
FORMAT_TERMINATOR1('h', 'x', unsigned short);
FORMAT_TERMINATOR1('h', 'X', unsigned short);
FORMAT_TERMINATOR1('h', 'u', unsigned short);

FORMAT_TERMINATOR2('l', 'l', 'd', long long);
FORMAT_TERMINATOR2('l', 'l', 'i', long long);

FORMAT_TERMINATOR2('l', 'l', 'o', unsigned long long);
FORMAT_TERMINATOR2('l', 'l', 'x', unsigned long long);
FORMAT_TERMINATOR2('l', 'l', 'X', unsigned long long);
FORMAT_TERMINATOR2('l', 'l', 'u', unsigned long long);

#define LETTER(str,pos) (pos < sizeof(str) ? str[pos] : 0)

#define PARSE(str)  \
	LETTER(str, 00), LETTER(str, 01), LETTER(str, 02), LETTER(str, 03), LETTER(str, 04),\
	LETTER(str, 05), LETTER(str, 06), LETTER(str, 07), LETTER(str,  8), LETTER(str,  9),\
	LETTER(str, 10), LETTER(str, 11), LETTER(str, 12), LETTER(str, 13), LETTER(str, 14),\
	LETTER(str, 15), LETTER(str, 16), LETTER(str, 17), LETTER(str, 18), LETTER(str, 19),\
	LETTER(str, 20), LETTER(str, 21), LETTER(str, 22), LETTER(str, 23), LETTER(str, 24),\
	LETTER(str, 25), LETTER(str, 26), LETTER(str, 27), LETTER(str, 28), LETTER(str, 29),\
    LETTER(str, 30), LETTER(str, 31), LETTER(str, 32), LETTER(str, 33), LETTER(str, 34),\
    LETTER(str, 35), LETTER(str, 36), LETTER(str, 37), LETTER(str, 38), LETTER(str, 39),\
    LETTER(str, 40), LETTER(str, 41), LETTER(str, 42), LETTER(str, 43), LETTER(str, 44),\
    LETTER(str, 45), LETTER(str, 46), LETTER(str, 47), LETTER(str, 48), LETTER(str, 49)

#define static_printf(format, ...)  _static_printf<Formatter<PARSE(format)>>(format, __VA_ARGS__)

template <class Formatter, class ...Args>
int _static_printf(const char *format, const Args&... args)
{
    Formatter f;
    string s = f.text(args...);
    fputs(s.c_str(), stdout);
    return (int)s.length();
}

int main(int argc, char **argv)
{
    string s = "govno";
    int i = 13, percent = 65;
    long long number = 1LL << 56;
    double epsilon = 0.001;
    long double huge = 1e308;
    const wchar_t *text = L"WOA!";
    const char *another = "text text";
    vector<float> vec{ 1,2,3 };
    list<int> l{ 5,6,7,8,9,0 };

    static_printf("%d %s %f %ls\n", i, s, 3.14f, text);
    static_printf("pointer points to %p address\n", &i);
    static_printf("invalid epsilon value %.4f which is %d%%\n", epsilon, percent);
    static_printf("custom message \"%s\" and some value %08X\n", "message", i);
    static_printf("long number %016llx and %llo and a %Lg\n", number, number, huge);
    static_printf("%s\n", another);
    static_printf("vector %v.2f and list %v2i\n", vec, l);
    //static_printf("%d %s %f", 1, 0, 2); // compilation error
    //static_printf("%d", 56.7e10); // warning
    return 0;
}
