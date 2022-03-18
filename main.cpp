#include "deque.h"

int main() {
    Deque<int, 4> d1(5, 111);
//    for (size_t i = 0; i < 5; ++i) {
//        d1.push_back(1);
//        d1.push_back(1);
//        d1.pop_back();
//        d1.push_front(1);
//        d1.pop_front();
//    }
//    d1.erase(d1.begin());
//    d1.erase(d1.begin());
//    d1.erase(d1.begin());
//    d1.erase(d1.begin());
//    d1.erase(d1.begin());
//    d1.erase(d1.begin());
//    d1.insert(d1.end() + 1,1);
//    Deque<int, 2> d1(2, 5);
//    d1 = d2;
//    std::cout << d2.at(1) << "";
//    std::cout<< d1.size();
//    for (auto it = d1.cbegin(); it != d1.end(); ++it){
//            std::cout << *it << "";
//    };
    auto it2 = d1.begin();
    auto it = d1.begin();
//    std::cout<< ((it2+0)-1 == (it-=1));
    auto r1 = it2 + 2;
    auto r2 = it - 3;
    bool t =((it2+0)-1 == (it-=1));
    std::cout<< '\n'<<t <<'\n';
//    it2 += 0;
//    std::cout<<'\n'<< (d1[0] == *d1.begin())<<'\n';
//    for (typename Deque<int, 4>::iterator it = d1.begin(); it != d1.end(); ++it) {
//        typename Deque<int, 4>::iterator it2 = it;
//        ++it2;
//    it->
//        std::cout << *it << " ";
//    }
//    Deque<int, 2>::const_iterator cit = d1.begin();

}