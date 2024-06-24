#include <algorithm>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <map>
#include <queue>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

const int Fu_size=5; // 功能部件数量
int opt = 0; // 指令数量
int finishNum=0; // 已完成的指令数量
int clockCycle=0; // 时钟周期

enum OP { Op_Null, L_D = 1, ADD_D, SUB_D, MULT_D, DIV_D }; // Op_Null:未定义操作
string ReOP[] = {"","L.D","ADD.D","SUB.D","MULT.D","DIV.D"};
enum FU { Fu_Null, Integer = 1, Mult1, Mult2, Add, Divide }; // Fu_Null:未定义功能部件
string ReFU[] = {"", "Integer", "Mult1", "Mult2", "Add", "Divide"};
enum Status {UnStart=0,IS,RO,EXB,EXE,WR}; // UnStart:未开始
int exeT[] = {0,1,2,2,10,40}; // 浮点执行所需时钟周期

struct InstructionStatus{
    OP Op = OP::Op_Null;
    string D = "", S1 = "", S2 = "";
    int Imm;
    int StatusTable[6] = {0}; // StatusTable:记录每个阶段的时钟周期
    Status status=Status::UnStart; // status:当前阶段
    bool finished = false,run=false; // finished:是否执行完成 run:该时钟周期是否执行
    FU Fu=Fu_Null; // Fu:记录指令对应的操作部件
};
vector<InstructionStatus> instructionStatus; // 指令状态表

struct FunctionUnit{
    bool busy = false;
    OP Op = OP::Op_Null;
    string Fi = "", Fj = "", Fk = "";
    FU Qj = FU::Fu_Null, Qk = FU::Fu_Null;
    bool Rj = false, Rk = false;
} FuStatus[Fu_size + 1]; // 功能部件状态表

map<string, FU> RegStatus; // 寄存器状态表
// 自定义比较函数，用于按照寄存器名的编号排序
bool compareRegisters(const std::pair<std::string, FU> &a, const std::pair<std::string, FU> &b)
{
    if (a.first[0] != b.first[0])
    {
        return a.first[0] < b.first[0];
    }
    // 提取寄存器编号
    int aNum = std::stoi(a.first.substr(1)); // 跳过首字符
    int bNum = std::stoi(b.first.substr(1));
    return aNum < bNum;
}

void PrintStatus(){ // 输出
    cout  << endl << "Clock Cycle " << clockCycle << endl;
    // 指令状态表
    cout << "------------------------------Instruction Status------------------------------" << endl;
    cout << setiosflags(ios::left) << setw(15) << "Instruction" << resetiosflags(ios::left) << setiosflags(ios::right) << setw(15) << "Issue" << setw(15) << "Read Operands" << setw(15) << "Execution" << setw(15) << "Write Result" << resetiosflags(ios::right) << endl;
    cout << "------------------------------------------------------------------------------" << endl;
    for (int i = 0; i < instructionStatus.size() ;i++){
        string ins = "", Ex;
        if (instructionStatus[i].Op == OP::L_D)
        {
            ins =
                ReOP[instructionStatus[i].Op] + " " + instructionStatus[i].D + " " + std::to_string(instructionStatus[i].Imm) + "(" + instructionStatus[i].S2 + ")";
            
        }
        else{
            ins =
                ReOP[instructionStatus[i].Op] + " " + instructionStatus[i].D + " " + instructionStatus[i].S1 + " " + instructionStatus[i].S2;
        }
        Ex =
            std::to_string(instructionStatus[i].StatusTable[Status::EXB]) + (instructionStatus[i].StatusTable[Status::EXB] > 0 ? "-" : "") + (instructionStatus[i].StatusTable[Status::EXE] > 0 ? std::to_string(instructionStatus[i].StatusTable[Status::EXE]) : " ");
        cout << setiosflags(ios::left) << setw(15) << ins << resetiosflags(ios::left) << setiosflags(ios::right) << setw(15) << instructionStatus[i].StatusTable[Status::IS] << setw(15) << instructionStatus[i].StatusTable[Status::RO] << setw(15) << Ex << setw(15) << instructionStatus[i].StatusTable[Status::WR] << resetiosflags(ios::right) << endl;
    }

    // 功能部件状态表
    cout << "------------------------------------------------------------------------------" << endl;
    cout << "-----------------------------Funtion Unit Status------------------------------" << endl;
    cout << setiosflags(ios::left) << setw(10) << "FU" << resetiosflags(ios::left) << setiosflags(ios::right) << setw(6) << "busy" << setw(6) << "Op" << setw(6) << "Fi" << setw(6) << "Fj" << setw(6) << "Fk" << setw(10) << "Qj" << setw(10) << "Qk" << setw(6) << "Rj" << setw(6) << "Rk" << resetiosflags(ios::right) << endl;
    cout << "------------------------------------------------------------------------------" << endl;
    for (int i = FU::Integer; i <= FU::Divide; i++){
        cout << setiosflags(ios::left) << setw(10) << ReFU[i] << resetiosflags(ios::left) << setiosflags(ios::right) << setw(6) << (FuStatus[i].busy ? "yes" : "no") << setw(6) << ReOP[FuStatus[i].Op] << setw(6) << FuStatus[i].Fi << setw(6) << FuStatus[i].Fj << setw(6) << FuStatus[i].Fk << setw(10) << ReFU[FuStatus[i].Qj] << setw(10) << ReFU[FuStatus[i].Qk] << setw(6) << (FuStatus[i].Fj != "" ? (FuStatus[i].Rj ? "yes" : "no") : "") << setw(6) << (FuStatus[i].Fk != "" ? (FuStatus[i].Rk ? "yes" : "no") : "") << resetiosflags(ios::right) << endl;
    }

    // 结果寄存器状态表
    cout << "------------------------------------------------------------------------------" << endl;
    cout << "-------------------------------Register Status--------------------------------" << endl;
    std::vector<std::pair<std::string, FU>> regList(RegStatus.begin(), RegStatus.end());
    std::sort(regList.begin(), regList.end(), compareRegisters);
    const int columns = 7;
    for (int i = 0; i < regList.size(); i += columns)
    {
        cout << setiosflags(ios::left) << setw(5) << " " << resetiosflags(ios::left) << setiosflags(ios::right);
        for (int j = i; j < i + columns && j < regList.size(); ++j){
            cout << setw(10) << regList[j].first;
        }
        cout << resetiosflags(ios::right) << endl;
        cout << "------------------------------------------------------------------------------" << endl;
        cout << setiosflags(ios::left) << setw(5) << "FU" << resetiosflags(ios::left) << setiosflags(ios::right);
        for (int j = i; j < i + columns && j < regList.size(); ++j){
            cout << setw(10) << ReFU[regList[j].second];
        }
        cout << resetiosflags(ios::right) << endl;
        cout << "------------------------------------------------------------------------------" << endl;
    }
    cout << endl;
}

void getInstruction(){ // 读指令
    for (int i = 0; i < opt; i++){
        string ins;
        getline(cin, ins);
        InstructionStatus Ins;
        // 进行分隔
        istringstream iss(ins);
        string token;
        int j = 0;
        while (getline(iss, token, ' ')){
            if (token == "") continue; // 防止有多个空格
            switch (j++){
            case 0:
                if (token.find("L.D") != -1){
                    Ins.Op = OP::L_D;
                }
                else if (token.find("ADD.D") != -1){
                    Ins.Op = OP::ADD_D;
                }
                else if (token.find("SUB.D") != -1){
                    Ins.Op = OP::SUB_D;
                }
                else if (token.find("MULT.D") != -1){
                    Ins.Op = OP::MULT_D;
                }
                else if (token.find("DIV.D") != -1){
                    Ins.Op = OP::DIV_D;
                }
                break;
            case 1:
                Ins.D = token;
                if (RegStatus.count(Ins.D)==0)
                    RegStatus.insert(make_pair(Ins.D, FU::Fu_Null));
                break;
            case 2:
                if (Ins.Op == OP::L_D){ // L.D指令较为特殊，具有立即数
                    int leftPos = token.find("("), rightPos = token.find(")");
                    Ins.Imm = atoi(token.substr(0, leftPos).c_str());
                    Ins.S2 = token.substr(leftPos + 1, rightPos - leftPos - 1);
                    if (RegStatus.count(Ins.S2) == 0)
                        RegStatus.insert(make_pair(Ins.S2, FU::Fu_Null));
                }
                else{
                    Ins.S1 = token;
                    if (RegStatus.count(Ins.S1) == 0)
                        RegStatus.insert(make_pair(Ins.S1, FU::Fu_Null));
                }
                break;
            case 3:
                Ins.S2 = token;
                if (RegStatus.count(Ins.S2) == 0)
                    RegStatus.insert(make_pair(Ins.S2, FU::Fu_Null));
            }
            if (Ins.S2 != "") break;
        }
        instructionStatus.push_back(Ins);
        // cout << i << " Op:" << Ins.Op << " D:" << Ins.D << " S1:" << Ins.S1 << " S2:" << Ins.S2 << " Imm:" << Ins.Imm << endl;
    }
}

void issue(InstructionStatus &Ins){ // 判断流出
    switch (Ins.Op){
        case OP::L_D:
            if (FuStatus[FU::Integer].busy == false && (RegStatus.count(Ins.D) == 0 || RegStatus[Ins.D] == Fu_Null)){ // 判断功能部件空闲且没有写后写（WAW）冲突
                Ins.run = true;
                Ins.status = Status::IS;
                Ins.Fu = FU::Integer;
            }
            break;
        case OP::ADD_D:
        case OP::SUB_D:
            if (FuStatus[FU::Add].busy == false && (RegStatus.count(Ins.D) == 0 || RegStatus[Ins.D] == Fu_Null)){ // 判断功能部件空闲且没有写后写（WAW）冲突
                Ins.run = true;
                Ins.status = Status::IS;
                Ins.Fu = FU::Add;
            }
            break;
        case OP::MULT_D:
            if (FuStatus[FU::Mult1].busy == false && (RegStatus.count(Ins.D) == 0 || RegStatus[Ins.D] == Fu_Null)){ // 判断功能部件空闲且没有写后写（WAW）冲突
                Ins.run = true;
                Ins.status = Status::IS;
                Ins.Fu = FU::Mult1;
            }
            else if (FuStatus[FU::Mult2].busy == false && (RegStatus.count(Ins.D) == 0 || RegStatus[Ins.D] == Fu_Null)){ // 判断功能部件空闲且没有写后写（WAW）冲突
                Ins.run = true;
                Ins.status = Status::IS;
                Ins.Fu = FU::Mult2;
            }
            break;
        case OP::DIV_D:
            if (FuStatus[FU::Divide].busy == false && (RegStatus.count(Ins.D) == 0 || RegStatus[Ins.D] == Fu_Null)){ // 判断功能部件空闲且没有写后写（WAW）冲突
                Ins.run = true;
                Ins.status = Status::IS;
                Ins.Fu = FU::Divide;
            }
            break;
    }
}

void runIssue(InstructionStatus &Ins){ // 执行流出
    Ins.StatusTable[Status::IS] = clockCycle;
    FuStatus[Ins.Fu].busy = true;
    FuStatus[Ins.Fu].Op = OP::L_D;
    FuStatus[Ins.Fu].Fi = Ins.D;
    FuStatus[Ins.Fu].Fj = Ins.S1;
    FuStatus[Ins.Fu].Fk = Ins.S2;
    FuStatus[Ins.Fu].Qj = RegStatus.count(FuStatus[Ins.Fu].Fj) == 0 ? FU::Fu_Null : RegStatus[FuStatus[Ins.Fu].Fj];
    FuStatus[Ins.Fu].Qk = RegStatus.count(FuStatus[Ins.Fu].Fk) == 0 ? FU::Fu_Null : RegStatus[FuStatus[Ins.Fu].Fk];
    FuStatus[Ins.Fu].Rj = FuStatus[Ins.Fu].Qj == FU::Fu_Null ? true : false;
    FuStatus[Ins.Fu].Rk = FuStatus[Ins.Fu].Qk == FU::Fu_Null ? true : false;
    RegStatus.find(Ins.D)->second = Ins.Fu;
}

void readOperand(InstructionStatus &Ins){ // 判断读操作数
    if (FuStatus[Ins.Fu].Rj && FuStatus[Ins.Fu].Rk)
    {
        Ins.run = true;
        Ins.status = Status::RO;
    }
}

void runReadOperand(InstructionStatus &Ins){ // 执行读操作数
    Ins.StatusTable[Status::RO] = clockCycle;
    FuStatus[Ins.Fu].Rj = false;
    FuStatus[Ins.Fu].Rk = false;
    FuStatus[Ins.Fu].Qj = FU::Fu_Null;
    FuStatus[Ins.Fu].Qk = FU::Fu_Null;
}

void Execution(InstructionStatus &Ins){ // 执行
    if(Ins.status==Status::RO){ // 判断执行阶段开始
        Ins.status = Status::EXB;
        Ins.StatusTable[Status::EXB] = clockCycle;
    }
    if (clockCycle - Ins.StatusTable[Status::EXB]+1==exeT[Ins.Op]){ // 判断执行阶段结束
        Ins.status = Status::EXE;
        Ins.StatusTable[Status::EXE] = clockCycle;
    }
}

void writeResult(InstructionStatus &Ins){ // 判断写结果
    bool flag=true;
    for(int i=FU::Integer;i<=FU::Divide;i++){
        if(i!=Ins.Fu){
            if ((FuStatus[i].Fj == FuStatus[Ins.Fu].Fi && FuStatus[i].Rj == true) || (FuStatus[i].Fk == FuStatus[Ins.Fu].Fi && FuStatus[i].Rk == true)){
                flag=false;
            }
        }
    }
    if(flag){
        Ins.run = true;
        Ins.status = Status::WR;
    }
}

void runWriteResult(InstructionStatus &Ins){ // 执行写结果
    Ins.StatusTable[Status::WR] = clockCycle;
    for (int i = FU::Integer; i <= FU::Divide; i++)
    {
        if (i != Ins.Fu)
        {
            if (FuStatus[i].Qj == Ins.Fu)
                FuStatus[i].Rj = true;
            if (FuStatus[i].Qk == Ins.Fu)
                FuStatus[i].Rk = true;
        }
    }
    RegStatus.find(Ins.D)->second = FU::Fu_Null;
    FuStatus[Ins.Fu].busy = false;
    FuStatus[Ins.Fu].Op = OP::Op_Null;
    FuStatus[Ins.Fu].Fi = "";
    FuStatus[Ins.Fu].Fj = "";
    FuStatus[Ins.Fu].Fk = "";
    FuStatus[Ins.Fu].Qj = FU::Fu_Null;
    FuStatus[Ins.Fu].Qk = FU::Fu_Null;
    Ins.finished=true;
    finishNum++;
}

int main()
{
    freopen("input.txt", "r", stdin);    // 输入文件
    freopen("output.txt", "w", stdout); // 输出文件
    cin >> opt;
    string str = "\n";
    getline(cin, str);
    getInstruction();
    PrintStatus();
    while(finishNum<opt){
        clockCycle++;
        bool issueFlag=false;
        for(int i=0;i<instructionStatus.size();i++){
            instructionStatus[i].run=false; // 设定运行情况初值
            if(!instructionStatus[i].finished){
                switch(instructionStatus[i].status){
                    case Status::UnStart:
                        issue(instructionStatus[i]);
                        issueFlag=true;
                        break;
                    case Status::IS:
                        readOperand(instructionStatus[i]);
                        break;
                    case Status::RO:
                    case Status::EXB:
                        Execution(instructionStatus[i]);
                        break;
                    case Status::EXE:
                        writeResult(instructionStatus[i]);
                        break;
                }
            }
            if(issueFlag) break;
        }
        for(int i=0;i<instructionStatus.size();i++){
            if(instructionStatus[i].run){
                switch (instructionStatus[i].status)
                {
                case Status::IS:
                    runIssue(instructionStatus[i]);
                    break;
                case Status::RO:
                    runReadOperand(instructionStatus[i]);
                    break;
                case Status::WR:
                    runWriteResult(instructionStatus[i]);
                    break;
                }
            }
        }
        PrintStatus();
    }
    return 0;
}