#include "PSRSimRank.h"
void PSRSimRank::run(int qv, int k){
	/*
	int A[]={1,2,3};
	int B[]={2};
	int Alen=3;
	int Blen=1;
	*/
	if(type==0)
		//Par_SR(A,Alen,B,Blen);
		Par_SR(qv, k);
	else if(type==1)
		PrunPar_SR(qv, k);
	else {
		printf("invalid method for PSR, valid ones are 0, 1.\n");
		return;		
	}
}
void PSRSimRank::Par_SR(int qv, int topk){
	#ifdef DEBUG
		printf("in Method Par_SR\n");	
	#endif
	int k=maxSteps;
	double** u =new double* [k+1];
	for(int i=0;i<k+1;i++){
		u[i] = new double[maxVertexId];
	}

	double **v= new double* [2];
	v[0]= new double [maxVertexId];
	v[1]= new double [maxVertexId];
	
	//***************************
	vector<double> d;
	d.resize(maxVertexId);
	for(int nodeId = 0;nodeId < maxVertexId; nodeId++){
		int os = orig_graphSrc[nodeId];
		int oe = orig_graphSrc[nodeId + 1];
		int xDeg = oe - os;
		if(xDeg == 0)
			d[nodeId] = 1;
		else
			d[nodeId] = 1.0 * (xDeg - 1) / xDeg;
	}
	//***************************
	//method body
//	for(int j=0;j<Blen;j++){
	for(int i=0;i<k+1;i++)
		memset(u[i],0,sizeof(double)*maxVertexId);
		//initialize u[l]
	u[0][qv]=1;
	for(int l=1;l<k+1;l++){
		//compute u[l] using u[l-1]
		for(int i=0;i<maxVertexId;i++){
			//compute u[l][i]
			int os=orig_graphSrc[i], oe = orig_graphSrc[i+1];
			for( int x=os;x<oe;x++){
				int temp=orig_graphDst[x];
				int xDeg=graphSrc[temp+1]-graphSrc[temp];
				u[l][i]+=1.0/xDeg *u[l-1][temp];
			}
				
		}
	}
	//use u[l] to compute v[k]
	for(int i=0;i<maxVertexId;i++)
		v[0][i] = d[i] * u[k][i];
	
	for(int l=1;l<k+1;l++){
		//use v[l-1] to compute v[l], use v[1-(l&1)] to compute v[l&1]
		//memset(v[l&1],0,sizeof(double)*maxVertexId);
		for(int i=0;i<maxVertexId;i++){
			v[l&1][i] = d[i] * u[k-l][i];
			int rs=graphSrc[i], re=graphSrc[i+1];
			int iDeg=re-rs;
			for(int x=rs;x<re;x++){
				v[l&1][i] += decayFactor*1.0/iDeg *v[1-(l&1)][graphDst[x]];
            }
        }
		//delete [] u[k-l];
		#ifdef DEBUG
		printf("v[%d] got:", l);
		for(int i=0;i<maxVertexId;i++)
			printf("%lf ",v[l&1][i]);
		printf("\n");
		#endif

	}
		// compute S[k]_j = (1-c)*v[k&1]
//	for(int a=0;a< maxVertexId;a++)
//		printf("(%d %d):%lf\n ",qv ,a , (1-decayFactor)*v[k&1][a]);
//	printf("\n");
	
	vector<SimRankValue> res;
  	res.resize(maxVertexId);
	for(int i = 0; i < maxVertexId; ++i){
		res[i].setVid(i);
	//	res[i].setValue((1-decayFactor)*v[k&1][i]);
		res[i].setValue(v[k & 1][i]);
		if(qv == i)// qv is most similar to itself
			res[i].setValue(0);
	}
   // printf("candidate size=%d\n", res.size());
	save(res, topk);
	
	for(int i=0;i<k+1;i++)
		delete [] u[i];
	delete [] u;
	delete [] v[0];
	delete [] v[1];
	delete [] v;
	
}
void PSRSimRank::PrunPar_SR(int qv, int topk){
	#ifdef DEBUG
		printf("in Method PrunPar_SR\n");
	#endif
	int k=maxSteps;
        double** u =new double* [k+1];
        for(int i=0;i<k+1;i++){
                u[i] = new double[maxVertexId];
        }

        double **v= new double* [2];
        v[0]= new double [maxVertexId];
        v[1]= new double [maxVertexId];

        //method body
       
        for(int i=0;i<k+1;i++)
            memset(u[i],0,sizeof(double)*maxVertexId);
            //initialize u[l]
            u[0][qv]=1;
			for(int l=1;l<k+1;l++){
				//compute u[l] using u[l-1]
				for(int i=0;i<maxVertexId;i++){
					double temp =u[l-1][i];
					if(temp==0)
						continue;
					else 
					{
						int rs=graphSrc[i], re= graphSrc[i+1];
						int deg=re-rs;
						for(int x=rs;x<re;x++){
							u[l][graphDst[x]]+= temp* 1.0/deg;
						}
					}
				}
			#ifdef DEBUG
			printf("u[%d] got:",l);
			for(int i=0;i<maxVertexId;i++)
				printf("%lf ",u[l][i]);
			printf("\n");
			#endif
                }
            //use u[l] to compute v[k]
			for(int i=0;i<maxVertexId;i++)
				v[0][i]=u[k][i];
			for(int l=1;l<k+1;l++){
				//use v[l-1] to compute v[l], use v[1-(l&1)] to compute v[l&1]
				//memset(v[l&1],0,sizeof(double)*maxVertexId);
				for(int i=0;i<maxVertexId;i++)
					v[l&1][i]=u[k-l][i];
				for(int i=0;i<maxVertexId;i++){
					// v[l&1][i]=u[k-l][i];
					if(v[1-(l&1)][i]==0)
						continue;
					else
					{
						int os=orig_graphSrc[i], oe=orig_graphSrc[i+1];
						for(int x=os;x<oe;x++){
							int node=orig_graphDst[x];
							int iDeg=graphSrc[node+1]-graphSrc[node];
							v[l&1][node] += decayFactor*1.0/iDeg *v[1-(l&1)][i];
                        }        
					}
                }
                        //delete [] u[k-l];
			#ifdef DEBUG
			printf("v[%d] got:", l);
                        for(int i=0;i<maxVertexId;i++)
                                printf("%lf ",v[l&1][i]);
                        printf("\n");
			#endif
            }
		// compute S[k]_j = (1-c)*v[k&1]
                
        
	vector<SimRankValue> res;
  	res.resize(maxVertexId);
	for(int i = 0; i < maxVertexId; ++i){
		res[i].setVid(i);
		res[i].setValue((1-decayFactor)*v[k&1][i]);
		if(qv == i)
			res[i].setValue(0);
	}
   // printf("candidate size=%d\n", res.size());
	save(res, topk);

        for(int i=0;i<k+1;i++)
                delete [] u[i];
        delete [] u;
        delete [] v[0];
        delete [] v[1];
        delete [] v;
}
