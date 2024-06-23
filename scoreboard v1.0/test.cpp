#include <iostream>
#include <ctime>
#include <fstream>

using namespace std;
int main()
{
    system("scoreboard.exe"); // 运行待测程序
    int result = system("fc answer.txt output.txt"); // 比对输出和答案是否一样
    if (result == 0){
        cout << "The files are identical." << endl;
    }
    else{
        cout << "The files are different." << endl;
    }
    system("pause");
    return 0;
}
