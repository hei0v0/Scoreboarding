#include <cstring>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>

using namespace std;

const int Fu_size=5; // 功能部件数量
int opt = 0; // 指令数量
bool finishNum=0; // 已完成的指令数量
int clockCycle=0;

enum OP { Op_Null, L_D = 1, ADD_D, SUB_D, MULT_D, DIV_D }; // Op_Null:未定义操作数
enum FU { Fu_Null, Integer = 1, Mult1, Mult2, Add, Divide }; // Fu_Null:未定义功能部件

struct Instruction{
    OP Op = OP::Op_Null;
    string D = "", S1 = "", S2 = "";
    int Imm;
};
struct InstructionStatus{
    Instruction ins;
    int Status[4] = {0},statusPos=-1;
    bool finished = false;
};
vector<InstructionStatus> instructionStatus; // 指令状态表
queue<Instruction> instruction;              // 指令队列

struct FunctionUnit{
    bool busy = false;
    OP Op = OP::Op_Null;
    string Fi = "", Fj = "", Fk = "";
    FU Qj = FU::Fu_Null, Qk = FU::Fu_Null;
    bool Rj = false, Rk = false;
} FuStatus[Fu_size + 1]; // 功能部件状态表

map<string,int> RegStatus; //寄存器状态表

void getInstruction(){
    for (int i = 0; i < opt; i++){
        string ins;
        getline(cin, ins);
        Instruction Ins;
        // 进行分隔
        istringstream iss(ins);
        string token;
        int j = 0;
        while (getline(iss, token, ' ')){
            if (token == "") continue; // 防止有多个空格
            cout << j << " " << token << ";" << endl;
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
                break;
            case 2:
                if (Ins.Op == OP::L_D){ // L.D指令较为特殊，具有立即数
                    int leftPos = token.find("("), rightPos = token.find(")");
                    Ins.Imm = atoi(token.substr(0, leftPos).c_str());
                    Ins.S2 = token.substr(leftPos + 1, rightPos - leftPos - 1);
                }
                else{
                    Ins.S1 = token;
                }
                break;
            case 3:
                Ins.S2 = token;
            }
            if (Ins.S2 != "") break;
        }
        instruction.push(Ins);
        // cout << i << " Op:" << Ins.Op << " D:" << Ins.D << " S1:" << Ins.S1 << " S2:" << Ins.S2 << " Imm:" << Ins.Imm << endl;
    }
}

void issue(){ // 判断流出
    if(!instruction.empty()){
        Instruction Ins=instruction.front();
        switch(Ins.Op){
            case L_D:
                if( FuStatus[FU::Integer].busy==false){

                }
        }
    }
}

int main()
{
    cin >> opt;
    string str = "\n";
    getline(cin, str);
    getInstruction();
    while(!(instruction.empty()&&finishNum==opt)){
        issue();
    }
    system("pause");
    return 0;
}