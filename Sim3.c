#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MaxComp 100
struct Comp{
    int id, Type, Eval, Value, InCount, OutCount, IsOut, Queued;
    struct Comp *Inputs[MaxComp], *Outputs[MaxComp];
};
struct Queue{
    struct Comp *Queued;
    struct Queue *Next, *Prev;
};
struct Circuit{
    int id, CCount;
    struct Comp *Comps[MaxComp];
    char InStr[MaxComp][MaxComp], OutStr[MaxComp][MaxComp];
    int InputStrCount, InputStrLen[MaxComp], OutputStrLen;
};

char Names[16][6] = {"", "INPUT", "OR", "AND", "XOR", "SRL"};

int CircuitCount = 0;
int CurrentCircuit = 0;
struct Circuit *Circuits[2];


struct Queue *Head = NULL;
struct Queue *Tail = NULL;

char OutFName[100];
FILE *OutputFile;

char VerFName[100];
FILE *VerFile;

char SettFName[100];
FILE *SettFile;

int VMode = 0;
int SMode = 0;

int Cycle = 0;
void LoadFile(char FName[100]);
void Run();
void AddQueue(struct Comp *ToAdd);
struct Comp *DeQueue();
void GetInputs();
void PCircuit(int CNum);
void ClearInputs();
void RemoveLastCircuit();
void CombineCircuits();
void WriteFile();
void VerifyOuts();
void Settings();

int main(int argc, char *argv[]){
    for(int i = 1; i<argc; i++){
        LoadFile(argv[i]);
    }
    int Option;
    while(1==1){
        printf("\n______________________\n");
        printf("\nCircuit defined: %s\nInputs cycles: %d\nOptions:\n1.Define inputs\n2.Clear inputs\n3.Print circuits\n4.Load file\n5.Remove last circuit\n6.Combine circuits\n7.Run\n8.Write circuit to file\n9.Settings\nOption:", (Circuits[0]!=NULL)?("yes"):("no"), (Circuits[0]==NULL)?(0):(Circuits[0]->InputStrCount));
        scanf("%d", &Option);
        switch(Option){
            case 1:
                GetInputs();
                break;
            case 2:
                ClearInputs();
                break;
            case 3:
                for(int i = 0; i<CircuitCount; i++){
                    PCircuit(i);
                }
                break;
                
            case 4:
                char Name[100];
                printf("\nEnter file name:");
                scanf("%s", Name);
                LoadFile(Name);
                break;
            case 5:
                RemoveLastCircuit();
                break;
            case 6:
                CombineCircuits();
                break;
            case 7:
                for(Cycle = 0; Cycle<Circuits[0]->InputStrCount; Cycle++){
                    for(CurrentCircuit = 0; CurrentCircuit<CircuitCount; CurrentCircuit++){
                        char Line[100];
                        memset(Line, 0, sizeof(Line));
                        Run();
                        strcat(Line, Circuits[CurrentCircuit]->InStr[Cycle]);
                        strcat(Line, ":");
                        strcat(Line, Circuits[CurrentCircuit]->OutStr[Cycle]);
                        strcat(Line, "\n");
                        printf("Result (cycle %d): %s", Cycle,Line);
                
                        if(OutFName[0] != '\0' && VMode == 0){
                            OutputFile = fopen(OutFName, "a");
                            if(Circuits[CurrentCircuit] != NULL){
                                fprintf(OutputFile, Line);
                            }
                            fclose(OutputFile);
                            
                        }
                        
                    }
                }
                break;
            case 8:
                WriteFile();
                break;

            case 9:
                Settings();
                break;
            case 10:
                printf("\nEnter output file:");
                scanf("%s", OutFName);
                VerFile = fopen(OutFName, "r");
                VerifyOuts();
                break;
            case 11:
                break;

        }
    }

}

void LoadFile(char FName[100]){
    FILE *File;
    File = fopen(FName, "r");
    if(File == NULL){
        printf("\nERROR: file does not exist");
        return;
    }
    char Line[100];
    int InCompCount = 0;
    fgets(Line, 100, File);
    if(strcmp(Line, "_CircuitFile_\n") != 0){
        printf("Error: not a circuit file");
        return;
    }
    struct Circuit *New = (struct Circuit*) malloc(sizeof(struct Circuit));
    if(New == NULL){
        printf("\nERROR: could not create struct");
    }
    printf("New circuit created with circuit id: %d (from: %s)\n", CircuitCount, FName);
    New->CCount = 0;
    New->InputStrCount = 0;
    New->id = CircuitCount;
    Circuits[CircuitCount] = New;
    CircuitCount = CircuitCount+1;

    while(fgets(Line, 100, File)){
        struct Comp *NewComp = (struct Comp*) malloc(sizeof(struct Comp));
        New->Comps[New->CCount] = NewComp;
        New->CCount = New->CCount+1;
        NewComp->Type = (int)Line[0]- (int)'0';
        NewComp->Eval = (NewComp->Type == 1)?(1):(0);
        //NewComp->id = 10*((int)Line[1]-(int)'0')+((int)Line[2]-(int)'0');
        NewComp->id = InCompCount;
        InCompCount = InCompCount+1;
        NewComp->IsOut = (int)Line[4]-(int)'0';
        NewComp->InCount = 0;
        NewComp->OutCount = 0;
        NewComp->Queued = 0;
        NewComp->IsOut = (int)Line[4]-(int)'0';
        for(int i = 6; Line[i]!=0;){
            int InID = (10*((int)Line[i]-(int)'0')) + (int)Line[i+1]-(int)'0';
            struct Comp *InFrom = New->Comps[InID];
            if(InFrom == NULL){
                printf("\nERROR: reference to input not valid");
                free(New);
                CircuitCount = CircuitCount-1;
                return;

            }

            NewComp->Inputs[NewComp->InCount] = InFrom;
            NewComp->InCount = NewComp->InCount+1;
            InFrom->Outputs[InFrom->OutCount] = NewComp;
            InFrom->OutCount = InFrom->OutCount+1;
        

            i = i+3;
        }
    }
    if(CircuitCount!=1){
        CombineCircuits();
    }

}

void RemoveLastCircuit(){
    CircuitCount = CircuitCount-1;
    return;
}


void Run(){
    char Line[200];
    memset(Line, 0, sizeof(Line));
    memset(Circuits[CurrentCircuit]->OutStr[Cycle], 0, sizeof(Circuits[CurrentCircuit]->OutStr[Cycle]));
    struct Circuit *ToRun = Circuits[CurrentCircuit];
    int StrPos = 0;
    for(int i = 0; ToRun->Comps[i]!=NULL; i++){
        if(ToRun->Comps[i]->Type == 1){
            ToRun->Comps[i]->Value = (ToRun->InStr[Cycle][StrPos]=='1')?(1):(0);
            ToRun->Comps[i]->Eval = 1;
            StrPos = StrPos+1;
            for(int OutId = 0; OutId<ToRun->Comps[i]->OutCount; OutId++){
                AddQueue(ToRun->Comps[i]->Outputs[OutId]);
            }
        }
        else{
            ToRun->Comps[i]->Eval = 0;
        }
    }
    memset(Circuits[CurrentCircuit]->OutStr[Cycle], 0, sizeof(Circuits[CurrentCircuit]->OutStr[Cycle]));
    
    int Steps = 0;
    if(SMode == 1){
        printf("\nRunning circuit %d (input %s)", CurrentCircuit, ToRun->InStr[Cycle]);
    }
    while(1==1){
        struct Comp *Current = DeQueue();
        if(Current == NULL){
            break;
        }
        if(Current->Eval == 1 || Current->Type == 1){
            continue;
        }
        int Val;
        int Eval = 1;
        int Even = 1;
        //Done allows to skip evaluating all inputs in cases, ie a single HIGH for an OR will return HIGH regardless of other inputs
        int Done = 0;
        switch(Current->Type){
            case 2:
                Val = 0;
                break;
            case 3:
                Val = 1;
                break;
            case 4:
                Val = 0;
                break;
            case 5:
                if(Current->Inputs[0]!=NULL && Current->Inputs[1]!=NULL){
                    if(Current->Inputs[0]->Value == 1 && Current->Inputs[1]->Value == 0 ){
                        Val = 1;
                    }
                    if(Current->Inputs[0]->Value == 0 && Current->Inputs[1]->Value == 1 ){
                        Val = 0;
                    }
                    else{
                        Val = Current->Value;
                    }
                }
                Eval = 1;
                Done = 1;
        }
        for(int i = 0; i<Current->InCount && Done == 0; i++){
            if(Current->Inputs[i]->Eval == 0){
                Eval = 0;
                break;
            }
            switch(Current->Type){
                case 2:
                    if(Current->Inputs[i]->Value == 1){
                        Val = 1;
                        Done = 1;
                    }
                    break;
                case 3:
                    if(Current->Inputs[i]->Value == 0){
                        Val = 0;
                        Done = 1;
                    }
                    break;
                case 4:
                    if(Current->Inputs[i]->Value == 1){
                        Val = (Val==0)?(1):(0);
                    }
                    break;
                 
            }
        }
        if(Eval == 1){
            Current->Eval = 1;
            Current->Value = Val;
            int OutputsEvaled = 1;
            if(SMode==1){
                printf("\n\tComp id: %d\n\tComp type: %s\n\tValue: %d\n\tSkipped:%d\n", Current->id, Names[Current->Type], Current->Value, Done);
            }
            for(int i = 0; i<Current->OutCount; i++){
                AddQueue(Current->Outputs[i]);
            }
        }
        Steps = Steps+1;
    }
    if(SMode==1){
        printf("\nEvaluated in %d steps\n", Steps);
    }
    Circuits[CurrentCircuit]->OutputStrLen = 0;
    for(int i = 0; Circuits[CurrentCircuit]->Comps[i]!=NULL; i++){
        if(Circuits[CurrentCircuit]->Comps[i]->IsOut == 1){
            Circuits[CurrentCircuit]->OutStr[Cycle][Circuits[CurrentCircuit]->OutputStrLen] = (Circuits[CurrentCircuit]->Comps[i]->Value==1)?('1'):('0');
            Circuits[CurrentCircuit]->OutputStrLen = Circuits[CurrentCircuit]->OutputStrLen+1;
            if(CurrentCircuit<CircuitCount-1){
                struct Circuit *Next = Circuits[CurrentCircuit+1];
                Next->InStr[Cycle][Next->InputStrLen[Cycle]] = (Circuits[CurrentCircuit]->Comps[i]->Value==1)?('1'):('0');
                Next->InputStrLen[Cycle] = Next->InputStrLen[Cycle]+1;
            }
        }
    }
}



void AddQueue(struct Comp *ToAdd){
    if(ToAdd->Queued == 1){
        return;
    }
    ToAdd->Queued = 1;
    struct Queue *NewQueue = (struct Queue*) malloc(sizeof(struct Queue));
    NewQueue->Queued = ToAdd;
    if(Head == NULL || Tail == NULL){
        Head = NewQueue;
        Tail = NewQueue;
        return;
    }
    NewQueue->Prev = Tail;
    Tail->Next = NewQueue;
    Tail = Tail->Next;
    return;

}

struct Comp *DeQueue(){
    if(Tail == NULL){
        return NULL;
    }
    struct Comp *Ret = Tail->Queued;
    struct Queue *Last = Tail;
    Ret->Queued = 0;
    if(Tail == NULL){
        return NULL;
    }
    if(Tail->Prev == NULL){
        Tail = NULL;
        Head = NULL;
        return Ret;
    }
    Tail->Prev->Next = NULL;
    Tail = Tail->Prev;
    return Ret;
}

void GetInputs(){
    if(Circuits[0]==NULL){
        printf("\nError: define circuit first");
        return;
    }
    printf("\nEnter input for cycle %d:", Circuits[0]->InputStrCount);
    scanf("%s", Circuits[0]->InStr[Circuits[0]->InputStrCount]);
    Circuits[0]->InputStrCount = Circuits[0]->InputStrCount+1;

}

void ClearInputs(){
    for(int i = 0; i<MaxComp; i++){
        memset(Circuits[CurrentCircuit]->InStr[i], 0, sizeof(Circuits[CurrentCircuit]->InStr[i]));
    }
    Circuits[0]->InputStrCount = 0;
}


void PCircuit(int CNum){
    struct Circuit *Current = Circuits[CNum];
    if(Current==NULL){
        return;
    }
    printf("\n\nCircuit %d (%d components):", Current->id, Current->CCount);
    for(int i = 0; Current->Comps[i]!=NULL; i++){
        struct Comp *CurrComp = Current->Comps[i];
        printf("\n\n\tComponent id:%d\n\tComponent type:%s\n\tComponent is output: %d\n\tLast value:%d", i, Names[CurrComp->Type], CurrComp->IsOut, CurrComp->Value);
        printf("\n\tInputs from:");
        for(int j = 0; j<CurrComp->InCount; j++){
            printf(" %d", CurrComp->Inputs[j]->id);
        }
        printf("\n\tOutputs to:");
        for(int j = 0; j<CurrComp->OutCount; j++){
            printf(" %d", CurrComp->Outputs[j]->id);
        }
    }
}




void CombineCircuits(){
    struct Circuit *Comb = Circuits[0];
    for(int i = 1; Circuits[i]!=NULL; i++){
        printf("\nCombining circuit %d", i);
        struct Circuit *Current = Circuits[i];
        for(int j = 0; Circuits[i]->Comps[j]!=NULL; j++){
            struct Comp *CurrComp = Current->Comps[j];
            if(Current->Comps[j]->Type == 1){
                for(int k = 0; Comb->Comps[k]!=NULL; k++){
                    if(Comb->Comps[k]->IsOut==1){
                        for(int l = 0; CurrComp->Outputs[l] != NULL; l++){
                            for(int m = 0; CurrComp->Outputs[l]->Inputs[m]!=NULL; m++){
                                if(CurrComp->Outputs[l]->Inputs[m] == CurrComp){
                                    CurrComp->Outputs[l]->Inputs[m] = Comb->Comps[k];
                                    Comb->Comps[k]->Outputs[Comb->Comps[k]->OutCount] = CurrComp->Outputs[l]->Inputs[m];
                                    Comb->Comps[k]->OutCount = Comb->Comps[k]->OutCount+1;
                                }
                            }
                        }
                        printf("\n\tRemapping input %d (Circuit %d)", j, i);

                        Comb->Comps[k]->IsOut = 0;
                        break;

                    }
                }
            }
            else{
                Comb->Comps[Comb->CCount] = CurrComp;
                CurrComp->id = Comb->CCount-1;
                Comb->CCount = Comb->CCount+1;
            }
        }
    }
    CircuitCount = 1;
    printf("\nCircuits combined successfully.");
}


void WriteFile(){
    struct Circuit *Current = Circuits[0];
    if(Current==NULL){
        printf("\nERROR: circuit does not exist");
        return;
    }

    char FName[100];
    printf("\nEnter file name:");
    scanf("%s", FName);
    if(Current==NULL){
        return;
    }
    FILE *out;
    out = fopen(FName, "w");
    fprintf(out, "_CircuitFile_\n");

    for(int i = 0; Current->Comps[i]!=NULL; i++){
        char Line[100];
        int CompID = Current->Comps[i]->id;
        Line[0] = (char)(Current->Comps[i]->Type + (int)'0');

        Line[2] = (char) (CompID%10 + (int) '0');
        Line[1] = (char) ((CompID-(CompID%10))%10 + (int) '0');
        Line[3] = ' ';
        Line[4] = (char)(Current->Comps[i]->IsOut + (int) '0');
        Line[5] = ' ';
        int j;
        for(j = 0; Current->Comps[i]->Inputs[j]!=NULL; j++){
            int InId = Current->Comps[i]->Inputs[j]->id;
            Line[5+(j*3)] = ' ';
            Line[6+(j*3)] = (char) ((InId-(InId%10))%10 + (int) '0');
            Line[7+(j*3)] = (char) (InId%10 + (int) '0');
            
        }
        Line[8+((j-1)*3)] = '\n';
        fprintf(out, Line);
    }
    printf("Success: circuit written to file '%s'", FName);
    fclose(out);
}

void VerifyOuts(){
    if(VerFile == NULL){

    }
    printf("\nRedefifning inputs");
    char Line[100];
    memset(Line, 0, sizeof(Line));
    VMode = 1;
    Circuits[CurrentCircuit]->InputStrCount = 0;
    Cycle = 0;
    fgets(Line, 100, VerFile);
    if(strcmp(Line, "_ResFile_\n") != 0){
        printf("Error: not a valid result file");
        return;
    }
    ClearInputs();
    while(fgets(Line, 100, VerFile)){
        Circuits[CurrentCircuit]->InputStrLen[Circuits[CurrentCircuit]->InputStrCount] = 0;
        int i = 0;
        for(i = 0; Line[i]!=':'; i++){
            Circuits[CurrentCircuit]->InStr[Circuits[CurrentCircuit]->InputStrCount][i] = Line[i];
            Circuits[CurrentCircuit]->InputStrLen[Circuits[CurrentCircuit]->InputStrCount] = Circuits[CurrentCircuit]->InputStrLen[Circuits[CurrentCircuit]->InputStrCount]+1;
        }
        Circuits[CurrentCircuit]->InputStrCount = Circuits[CurrentCircuit]->InputStrCount+1;
        Run();
        int Valid = 1;
        printf("%s %s", Circuits[CurrentCircuit]->OutStr[Cycle], Line);

        if(Valid == 1){
            printf("Yes");
        }
        else{
            printf("No");
        }
        Cycle = Cycle+1;
    }
    VMode = 0;
}

void Settings(){
    int Option;
    while(1==1){
        printf("\nSettings\n1.SettingsFile: %s\n2.Output file: %s\n3.Steps mode: %d\n4.Exit\nOption:", SettFName, OutFName, SMode);
        scanf("%d", &Option);
        switch (Option) {
            case 1:
                printf("\nSettings file name:");
                scanf("%d", SettFName);
                break;
            case 2:
                printf("\nOutput file name:");
                scanf("%s", OutFName);
                if(OutFName[0]!='\0'){
                    OutputFile = fopen(OutFName, "w");
                    fprintf(OutputFile, "_ResFile_\n");
                    fclose(OutputFile);
                }
                else{
                    printf("\nNo output file selected");
                }
                break;
            case 3:
                SMode = (SMode==1)?(0):(1);
                printf("\nSteps mode set to %d", SMode);
                break;
            case 4:
                return;
    
        }
    }
    

}

void ReadSettingsFile(){

}