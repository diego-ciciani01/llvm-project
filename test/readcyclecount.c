unsigned long long bpf_tsc(void){
        return __builtin_readcyclecounter();
}

int main(void){
    bpf_tsc();
    return 0;
}
