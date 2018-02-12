#include "../../include/config.h"
#include "../../include/parameterSetting.h"
#define METHOD_NUM 40 // max number of method
#define MAX_TOPK 10000 // max k for topk
#define MAX_QUERY_NUM 1000 //max query number for query in file
//MAX_TOPK must be larger than the DEFAULT_TOPK specifiled in parameter.h
char outputpath[METHOD_NUM][100];
char method_name[METHOD_NUM][100];

int exact_vid[MAX_QUERY_NUM][MAX_TOPK];
double exact_val[MAX_QUERY_NUM][MAX_TOPK];

int appro_vid[MAX_QUERY_NUM][MAX_TOPK];
double appro_val[MAX_QUERY_NUM][MAX_TOPK];

int query_num;//global variable, to record how many queries in a single file.

int analysis_k = 20;
double get_NDCG(int k);//return the ndcg@k from appro[][]& exact[][], qv, k is read from input file
int findNode(int* topk, int node, int len);//find the index of node in topk[]
double get_Precision(int k);
double get_AvgDiff(int k);
bool readExactFile(char* filename);//stores in exact_vid[], exact_val[]
bool readApproFile(char* filename);//stores in appro_vid[], appro_val[]
//specify the config file
char config[100] = "../../config/wikiVoteC";
bool read_config();
bool topKWithout_1();

int exact_qv[MAX_QUERY_NUM];
int exact_topk[MAX_QUERY_NUM];
int appro_qv[MAX_QUERY_NUM];
int appro_topk[MAX_QUERY_NUM];

int main(){
	if(!read_config()){
		printf("%s\n", "error reading config file");
		return 0;
	}
	char category[100];
	sprintf(category, "../../dataset/%s/output", inpath);
	
	sprintf(method_name[0], "accur");
	sprintf(method_name[1], "OIP");
	sprintf(method_name[2], "naivePSR");
	sprintf(method_name[3], "fasterPSR");
	
	sprintf(method_name[4], "effiEVD");
	sprintf(method_name[5], "effiSVD");
	sprintf(method_name[6], "OptEffiSVD");
	sprintf(method_name[7], "originalKronsim");
	sprintf(method_name[8], "OptKronSim");
	
	sprintf(method_name[9], "topsim");
	sprintf(method_name[10], "trun_topsim");
	sprintf(method_name[11], "prio_topsim");
	
	sprintf(method_name[12], "srgs");
	sprintf(method_name[13], "srgs_usDisk");
	sprintf(method_name[14], "srgs_DiskCompress");
	
	sprintf(method_name[15], "www05");
	sprintf(method_name[16], "mod14");
	//method_name[0] = "accur";
	for(int i = 0; i <= 16;++i){
		sprintf(outputpath[i], "%s/%s", category, method_name[i]);
	}

	printf("%s\n", outputpath[0]);
	if(!readExactFile(outputpath[0])){
		printf("%s\n", "file cannot be opened.");
		return 0;
	}
	printf("%s    %s    %s    %s\n","method name", "precision", "NDCG", "AvgDiff");
	for(int fi = 1; fi <= 16; fi++){	
		if(!readApproFile(outputpath[fi])){
			printf("%s cannot be opened\n", outputpath[fi]);
			continue;
		}
		if(!topKWithout_1()){
			printf("%s\n", "topK is too large, please make topK smaller in config file or resample some test points.");
			continue;
		}
			
		double precision = get_Precision(analysis_k);
		double ndcg = get_NDCG(analysis_k);
		double avg_diff = get_AvgDiff(analysis_k);
	//	printf("%s\t", method_name[fi]);
		printf("%s    %lf    %lf    %lf\n", method_name[fi], precision, ndcg, avg_diff);
	}
	
	return 0;
}
bool topKWithout_1(){//whether 
	for(int i=0;i< query_num;++i){
		if(appro_topk[i] == exact_topk[i] && appro_qv[i] == exact_qv[i])
			continue;
		else
			return false;
	}
	return true;

}
bool readExactFile(char* filename){
	memset(exact_vid, 0, sizeof(exact_vid));
	memset(exact_val, 0, sizeof(exact_val));
	memset(exact_qv, 0, sizeof(exact_qv));
	memset(exact_topk, 0, sizeof(exact_topk));
	FILE* fp = fopen(filename, "rb");
	if(fp != NULL){
		int qv,k;
		query_num = 0;//to record the case ID
		while(fread(&qv, sizeof(int), 1, fp) != 0){
			fread(&k, sizeof(int), 1, fp);
			exact_qv[query_num] = qv;
			exact_topk[query_num] = k;
			for(int dataI = 0; dataI < k; dataI ++){
				fread(&exact_vid[query_num][dataI], sizeof(int), 1, fp);
				fread(&exact_val[query_num][dataI], sizeof(double), 1, fp);
			}
			query_num ++;
		}
		fclose(fp);
		return true;
	}
	else{
		return false;
	}
}
bool readApproFile(char* filename){
	memset(appro_vid, 0, sizeof(appro_vid));
        memset(appro_val, 0, sizeof(appro_val));
	memset(appro_topk, 0, sizeof(appro_topk));
	memset(appro_qv, 0, sizeof(appro_qv));
	
	FILE* fp = fopen(filename, "rb");
	if(fp != NULL){
		
		int qv,k;
		query_num = 0;//to record the case ID
		while(fread(&qv, sizeof(int), 1, fp) != 0){
			fread(&k, sizeof(int), 1, fp);
			appro_qv[query_num] = qv;
			appro_topk[query_num] = k;
		//	fread(&k, sizeof(int), 1, fp);
			for(int dataI = 0; dataI < k; dataI ++){
				fread(&appro_vid[query_num][dataI], sizeof(int), 1, fp);
				fread(&appro_val[query_num][dataI], sizeof(double), 1, fp);
			}
			query_num ++;
		}
		fclose(fp);
		return true;
	}
	else{
		return false;
	}
}
int findNode(int* topk, int node, int len){//if find node in topk[], return index, else return -1.
	for(int ti = 0; ti < len; ti++){
		if(topk[ti] == node){
			//printf("ti found is %d\n",ti);
			return ti;
		}
	}
	return -1;
}
double get_Precision(int k){
	int sum = 0;
	for(int qi = 0; qi < query_num; qi++){//query case
		for(int ti = 0; ti < k; ti++){//top 1, top2, top3...
			if(findNode(exact_vid[qi], appro_vid[qi][ti], k) != -1){//intersection of first k elements
				sum ++;
			//	printf("sum is; %d\n", sum);	
			}
		}
	}

	return 1.0 * sum / (k * query_num);
}
double get_AvgDiff(int k){
	double ad = 0.0;
	for(int qi = 0; qi < query_num; qi++){
		double tmp_exact = 0;
		for(int ti = 0; ti < k; ti++){
			tmp_exact += exact_val[qi][ti];
		}
		double tmp_appro = 0;;
		for(int ti = 0; ti < k; ti++){
			int exact_in_appro = findNode(appro_vid[qi], exact_vid[qi][ti], DEFAULT_TOPK);
			if(exact_in_appro != -1) 
				tmp_appro += appro_val[qi][exact_in_appro];
		}
		ad += tmp_exact - tmp_appro;
	}
	return ad / (query_num * k);
}
double get_NDCG(int k){
	double nc = 0.0;
	for(int qi = 0; qi < query_num; qi++){
		double tmp_exact = 0;
		for(int ti = 0; ti < k; ti++){
			tmp_exact += (pow(2, exact_val[qi][ti]) - 1)/(log(ti + 2)/log(2));
		}
		double tmp_appro = 0;
		for(int ti = 0; ti <k; ti++){
			int appro_in_exact = findNode(exact_vid[qi], appro_vid[qi][ti], DEFAULT_TOPK);
			if(appro_in_exact != -1) 
				tmp_appro += (pow(2, exact_val[qi][appro_in_exact]) -1)/(log(ti + 2)/log(2));
		}
		nc += tmp_appro / tmp_exact;
	}
	return nc / query_num;
}
bool read_config(){
	bool flag = true;
	FILE* fp = fopen(config, "r");
	if(fp == NULL)
		return false;
    	char line[1024];
	char key[128];
	char value[128];
    	char dummy[128];
	while(fgets(line, 1024, fp) != NULL){
        sscanf(line, "%s %s #%s", key, value, dummy);
        if(strcmp(key, "-qf") == 0) {
            queryInFile = (strcmp(key, "true") == 0 ? true : false);
        }
	else if(strcmp("-C", key) == 0){
		decayFactor = atof(value);
	}
	else if(strcmp("-topk", key) == 0){
		DEFAULT_TOPK = atoi(value);
	}
	else if(strcmp("-range", key) ==0){
		DEFAULT_RANGE = atof(value);
	}
        else if(strcmp(key, "-T") == 0) {
            numIter = atoi(value);
        }
        else if(strcmp(key, "-bi") == 0) {
            buildIndex = (strcmp(value, "true") == 0 ? true : false);
        }
         else if(strcmp(key, "-hi") == 0) {
            hasIndex = (strcmp(value, "true") == 0 ? true : false);
        }
        else if(strcmp(key, "-m") == 0){
            strcpy(method, value);
        }
        else if(strcmp(key, "-g") == 0) {
            strcpy(inpath, value);
        }
        else if(strcmp(key, "-en") == 0) {
            edgeNum = atoi(value);
        }
        else if(strcmp(key, "-vn") == 0) {
            verticesNum = atoi(value);
        }
        else if(strcmp(key, "-iv") == 0) {
            initValue = atof(value);
        }
        else if(strcmp(key, "-rank") == 0){
            Rank=atoi(value);
        }
        else if(strcmp(key, "-fm") == 0) {
            isFm = (strcmp(value, "true") == 0 ? true : false);
        }
	else if(strcmp("-sn", key) == 0){
		sampleNum = atoi(value);
	}
	else if(strcmp("-sqn", key) == 0){
		sampleQueryNum = atoi(value);
	}
        else if(strcmp(key, "-ud") == 0) {
            usDisk = atoi(value);
        }
        else if(strcmp(key, "-ts") == 0) {
            tsm_type = atoi(value);
        }
        else if(strcmp(key, "-es") == 0){
            effisim_type=atoi(value);
        }
        else if(strcmp(key, "-ks") == 0){
            kronsim_type=atoi( value);
        }
        else if(strcmp(key, "-psrt") == 0){
            psr_type = atoi(value);
        }
        else {
            printf("invalid parameter: %s\n", key);
	    flag = false;
            }
     }
	
	fclose(fp);
    	return flag;
}
