#include <iostream>
#include <cstring>

class String{
public:
    String() = default;
    String(const char* s){
        size_t len = strlen(s);
        us = len;
        al = (len<<1);
        str = new char[al];
//        strcpy(str,s);
        memcpy(str, s, len);
    }
    String(const char c){
        StringFromChar(c);
    }
    String(size_t n, char c): str(new char[(n<<1)]), us(n), al(n<<1){
        memset(str, c, n);
    }

    String(const String& s): str(new char[(s.us << 1)]), us(s.us), al((s.us<<1)){
        memcpy(str, s.str, us);
    }

    explicit String(size_t size){
        al = size;
        str = new char[al];
    }

    String& operator=(const String & s){
        String copy = s;
        swap(copy);
        return *this;
    }

    char& operator[](size_t index){ // return last el('/0') if index > us
        return str[index + (us - index) * (index > us)];
    }
    const char& operator[](size_t index) const{ // return last el('/0') if index > us
        return str[index + (us - index) * (index > us)];
    }

    friend bool operator==(const String& s1, const String& s2);
    friend std::istream & operator >> (std::istream & in, String &s);

    size_t length() const{
        return us;
    }

    void push_back(char c){
        if (str == nullptr){
            StringFromChar(c);
            return;
        }
        resize_expand(*this);
        str[us++] = c;
    }

    void pop_back(){
        --us;
//        resize_narrow(*this);
    }

    char& front(){
        return str[0];
    }
    char& back(){
        return str[us - 1];
    }
    const char& front() const{
        return str[0];
    }
    const char& back() const{
        return str[us - 1];
    }

    String& operator+=(const char *s){
        size_t lS = strlen(s);
        size_t newSz = (lS + us) << 1;
        if (newSz > al) resize_size(*this, newSz);
        memcpy(str + us, s, lS);
        us+=lS;
        return *this;
    }
    String& operator+=(const String& s){
        if (s.us + us > al) resize_size(*this, (s.us + us) << 1);
        memcpy(str + us, s.str, s.us);
        us+=s.us;
        return *this;
    }

    String& operator+=(const char c){
        resize_expand(*this);
        str[us++] = c;
        return *this;
    }

    size_t find(const char *sb) const{
        size_t len = strlen(sb);
        bool Nfound = 1;
        size_t i = 0;
        while (Nfound && i <= us - len){
            Nfound = 0;
            for (size_t j = 0; j < len; ++j) {
                if(str[i+j] != sb[j]){
                    Nfound = 1;
                    break;
                }
            }
            ++i;
        }
        return Nfound ? us : i-1;
    }

    size_t rfind(const char *sb) const{
        size_t len = strlen(sb);
        bool Nfound = 1;
        long long i = us - len - 1;
        while (Nfound && i >= 0){
            Nfound = 0;
            for (size_t j = 0; j < len; ++j) {
                if(str[i+j] != sb[j]){
                    Nfound = 1;
                    break;
                }
            }
            --i;
        }
        return Nfound ? us : i+1;
    }

    String substr(size_t start,size_t count) const{
        String s(count);
        memcpy(s.str,str + start, count);
        s.us = count;
        return s;
    }

    bool empty() const{
        return us == 0;
    }

    void clear(){
        delete[] str;
        str = nullptr;
        us = 0;
    }

    ~String(){
        delete[] str;
    }
private:
    char *str = nullptr;
    size_t us = 0;
    size_t al = 0;

    void StringFromChar(const char c){
        al = 16;
        us = 1;
        str = new char[al];
        str[0] = c;
    }

    void swap(String s1){
        std::swap(s1.str, str);
        std::swap(s1.us, us);
        std::swap(s1.al, al);
    }

//    static void resize_narrow(String &s) {
//        if (s.us * 4 < s.al) {
//            s.al >>= 1;
//            char * nS = new char[s.al];
//            memcpy(nS,s.str, s.us);
//            delete[] s.str;
//            s.str = nS;
//        }
//    }
    static void resize_size(String &s, size_t size) {
        s.al = size;
        char * nS = new char[s.al];
        memcpy(nS,s.str, s.us);
        delete[] s.str;
        s.str = nS;
    }
    static void resize_expand(String &s) {
        if (s.us * 2 > s.al) {
            resize_size(s, s.al << 1);
        }
    }

};

String operator+(const String& s1, const String& s2){
    size_t newSz = (s1.length() + s2.length());
    String s(newSz);
    s+=s1;
    s+=s2;
    return s;
}

bool operator==(const String& s1, const String& s2){
    if(s1.us != s2.us) return false;
    for (size_t i = 0; i < s1.us; ++i) {
        if(s1[i] != s2[i]) return false;
    }
    return true;
}

std::ostream & operator << (std::ostream & cout,const String &s){
    for (size_t i = 0; i < s.length(); ++i) {
        cout<<s[i];
    }
    return cout;
}

std::istream & operator >> (std::istream & in, String &s){
    char c;
    in >> std::noskipws;
    while (in >> c && c != '\n'){
        s.push_back(c);
    }
    std::cin>>std::skipws;
    return in;
}
