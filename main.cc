/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <string>
#include <istream>
#include <vector>
#include <iomanip>                
#include "cache.h"
using namespace std;

void MSI(vector <Cache*> cacheArray, int proc_num, ulong address, char op, int num_proc)
{
	//For NULL State
	if (cacheArray[proc_num]->findLine(address) == NULL)
	{	//PrRd --- For Read : NULL ---> SHARED | FOR Write : NULL ---> MODIFIED
		if (op == 'r')
		{
			cacheArray[proc_num]->Access(address, op);
			//cacheArray[proc_num]->num_of_mem_trans++;
			cacheArray[proc_num]->findLine(address)->setFlags(SHARED);
			//Generate BusRD to turn modified to shared
			for (int i = 0; i<num_proc; i++)
			{
				if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
				{
					if ((cacheArray[i]->findLine(address))->getFlags() == MODIFIED)
					{
						cacheArray[i]->num_of_flushes++;
						cacheArray[i]->writeBack(address);
						cacheArray[i]->num_of_interventions++;
						//cacheArray[i]->num_of_mem_trans++;
						(cacheArray[i]->findLine(address))->setFlags(SHARED);
					}
				}
			}
			return;
		}
		else
		{	//NULL ---> Modified
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			//cacheArray[proc_num]->num_of_mem_trans++;
			//Generates BusRDx to invalidate other caches
			for (int i = 0; i<num_proc; i++)
			{
				if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
				{
					if((cacheArray[i]->findLine(address)->getFlags())==MODIFIED)
					{
						//cacheArray[i]->num_of_mem_trans++;
					}
					(cacheArray[i]->findLine(address))->invalidate();
					cacheArray[i]->num_of_invalidations++;
					cacheArray[i]->num_of_flushes++;
					cacheArray[i]->writeBack(address);
					//cacheArray[i]->num_of_mem_trans++;
				}
			}
			return;
		}
	}
	//For INVALID State
	if ((cacheArray[proc_num]->findLine(address))->getFlags() == INVALID)
	{	//PrRd --- For Read : INVALID ---> SHARED | FOR Write : INVALID ---> MODIFIED
		if (op == 'r')
		{
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(SHARED);
			//cacheArray[proc_num]->num_of_mem_trans++;
			for (int i = 0; i<num_proc; i++)
			{
				if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
				{
					if ((cacheArray[i]->findLine(address))->getFlags() == MODIFIED)
					{
						cacheArray[i]->num_of_flushes++;
						cacheArray[i]->writeBack(address);
						cacheArray[i]->num_of_interventions++;
						//cacheArray[i]->num_of_mem_trans++;
						(cacheArray[i]->findLine(address))->setFlags(SHARED);
					}
					//(cacheArray[i]->findLine(address))->setFlags(SHARED);

				}
			}
			return;
		}
		else
		{	//Invalid ---> Modified
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			//cacheArray[proc_num]->num_of_mem_trans++;
			//Generates BusRDx to invalidate other caches
			for (int i = 0; i<num_proc; i++)
			{
				if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
				{
					if((cacheArray[i]->findLine(address)->getFlags())==MODIFIED)
					{
						//cacheArray[i]->num_of_mem_trans++;
					}
					(cacheArray[i]->findLine(address))->invalidate();
					cacheArray[i]->num_of_invalidations++;	
				}
			}
			return;
		}
	}

	//For SHARED State
	if ((cacheArray[proc_num]->findLine(address))->getFlags() == SHARED)
	{
		/*READ*/
		if (op == 'r')
		{
			cacheArray[proc_num]->Access(address, op);
			return;
		}
		/*WRITE*/			
		else
		{	//Shared  ----> Modified
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			cacheArray[proc_num]->num_of_mem_trans++;
			//Generates BusRDx to invalidate other caches
			for (int i = 0; i<num_proc; i++)
			{
				if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
				{
					if((cacheArray[i]->findLine(address)->getFlags())!=INVALID)
					{
						cacheArray[i]->num_of_invalidations++;
					    (cacheArray[i]->findLine(address))->invalidate();
					}
				}
			}
			return;
		}
	}

	//For MODIFIED State
	if ((cacheArray[proc_num]->findLine(address))->getFlags() == MODIFIED)
	{
		if (op == 'r')
		{
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			return;
		}
		else
		{
			cacheArray[proc_num]->Access(address, op);
			//cacheArray[proc_num]->num_of_mem_trans++;
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			return;
		}
	}

    return;
}

void MESI(vector <Cache*> cacheArray, int proc_num, ulong address, char op, int num_proc)
{
	//For INVALID State
	if (cacheArray[proc_num]->findLine(address) == NULL)
	{	//PrRd --- For Read(C) : INVALID ---> SHARED | For Read(!C) : INVALID ---> EXCLUSIVE | FOR Write : INVALID ---> MODIFIED
		bool C = false;
		for (int i = 0; i<num_proc; i++)
		{
			if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
			{
				C = true;
			}
		}
		if (op == 'r')
		{
			if (C == true)
			{
				cacheArray[proc_num]->Access(address, op);
				(cacheArray[proc_num]->findLine(address))->setFlags(SHARED);
				//Cache to Cache Transfer when Line exists in another processor and State INVALID-->SHARED
				cacheArray[proc_num]->num_of_cache_to_cache_transfer++;
				//Generate BusRD to turn modified to shared
				for (int i = 0; i<num_proc; i++)
				{
					if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
					{
						if ((cacheArray[i]->findLine(address))->getFlags() == MODIFIED)
						{
							cacheArray[i]->num_of_interventions++;
							cacheArray[i]->num_of_flushes++;
							cacheArray[i]->writeBack(address);
							//cacheArray[proc_num]->num_of_chache_to_cache_transfer++;
						}
						else if ((cacheArray[i]->findLine(address))->getFlags() == EXCLUSIVE)
						{
							cacheArray[i]->num_of_interventions++;
							//cacheArray[proc_num]->num_of_chache_to_cache_transfer++;

						}
						(cacheArray[i]->findLine(address))->setFlags(SHARED);

					}
				}
			}
			else
			{
				cacheArray[proc_num]->Access(address, op);
				(cacheArray[proc_num]->findLine(address))->setFlags(EXCLUSIVE);
			}
			return;
		}
		else
		{	//Invalid ---> Modified
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			//Generates BusRDx to invalidate other caches
			for (int i = 0; i<num_proc; i++)
			{
				if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
				{
					if ((cacheArray[i]->findLine(address))->getFlags() == SHARED)
					{
						cacheArray[i]->num_of_invalidations++;
						cacheArray[proc_num]->num_of_cache_to_cache_transfer++;
					}
					else if ((cacheArray[i]->findLine(address))->getFlags() == MODIFIED)
					{
						cacheArray[i]->num_of_flushes++;
						cacheArray[i]->writeBack(address);
						cacheArray[i]->num_of_invalidations++;
						cacheArray[proc_num]->num_of_cache_to_cache_transfer++;
					}
					else
					{
						cacheArray[i]->num_of_invalidations++;

					}
					(cacheArray[i]->findLine(address))->invalidate();
				}
			}
			return;
		}
	}
	//For Exclusive State
	if ((cacheArray[proc_num]->findLine(address))->getFlags() == EXCLUSIVE)
	{	//PrRd --- For Read : EXCLUSIVE ---> EXCLUSIVE | FOR Write : EXCLUSIVE ---> MODIFIED
		if (op == 'r')
		{
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(EXCLUSIVE);
			return;
		}
		else
		{	//EXCLUSIVE ---> MODIFIED
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			return;
		}
	}
	//For SHARED State
	if ((cacheArray[proc_num]->findLine(address))->getFlags() == SHARED)
	{
		/*READ*/			
		if (op == 'r')
		{
			cacheArray[proc_num]->Access(address, op);
			return;
		}
		/*WRITE*/			
		else
		{	//Shared  ----> Modified
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			//Generates BusUPGR to invalidate other caches
			for (int i = 0; i<num_proc; i++)
			{
				if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
				{
					if ((cacheArray[i]->findLine(address))->getFlags() == SHARED)
					{
						cacheArray[i]->num_of_invalidations++;
						//cacheArray[i]->num_of_chache_to_cache_transfer++;
					}
					else
					{
						cacheArray[i]->num_of_invalidations++;
					}
					(cacheArray[i]->findLine(address))->invalidate();
				}
			}
			return;
		}
	}
	//For MODIFIED State
	if ((cacheArray[proc_num]->findLine(address))->getFlags() == MODIFIED)
	{
		if (op == 'r')
		{
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			return;
		}
		else
		{
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			return;
		}
	}

    return;
}

void DRAGON(vector <Cache*> cacheArray, int proc_num, ulong address, char op, int num_proc)
{
	// state: null, Sc, Sm, E, Modified 
	
	// test if any copy exists
	bool C = false;
	for (int i = 0; i<num_proc; i++)
	{
		if (i != proc_num && cacheArray[i]->findLine(address) != NULL && cacheArray[i]->findLine(address)->getFlags() != INVALID)
		{
			C = true;  
		}
	}

	// state null, read: null -> SharedClean(C), Exclusive(!C) write: null -> SharedWrite(C), Modified(!C)
	if(cacheArray[proc_num]->findLine(address) == NULL || cacheArray[proc_num]->findLine(address)->getFlags() == INVALID)
	{
		if(op=='r')
		{
			cacheArray[proc_num]->Access(address, op);
            if(C==true)
            {
                
                	(cacheArray[proc_num]->findLine(address))->setFlags(SHARED_CLEAN);
                
                    for (int i = 0; i<num_proc; i++)
				    {
					    if (i != proc_num && cacheArray[i]->findLine(address) != NULL && cacheArray[i]->findLine(address)->getFlags() != INVALID)
					    {
						    if ((cacheArray[i]->findLine(address))->getFlags() == MODIFIED)
						    {
							    cacheArray[i]->num_of_interventions++;
							    cacheArray[i]->num_of_flushes++;
							    cacheArray[i]->writeBack(address); // write back 
							    (cacheArray[i]->findLine(address))->setFlags(SHARED_MODIFIED);
							    continue;
						    }
						    if ((cacheArray[i]->findLine(address))->getFlags() == EXCLUSIVE)
						    {
							    cacheArray[i]->num_of_interventions++;
							    (cacheArray[i]->findLine(address))->setFlags(SHARED_CLEAN);
						    }
						    if ((cacheArray[i]->findLine(address))->getFlags() == SHARED_MODIFIED)
                			{
                				cacheArray[i]->num_of_flushes++; // snoop a busRd, flush, according to FSM
                				cacheArray[i]->writeBack(address); // write back
                			}
					    }
				    }
                
            } 
            else
            {
                // no copy, set exclusive
                //cacheArray[proc_num]->num_of_mem_trans++;
                (cacheArray[proc_num]->findLine(address))->setFlags(EXCLUSIVE);
            }
            return;
		}
		else
		{
			// now it's write

			cacheArray[proc_num]->Access(address, op);
			if(C==true)
			{
				(cacheArray[proc_num]->findLine(address))->setFlags(SHARED_MODIFIED);
				for (int i = 0; i<num_proc; i++)
				{
					if (i != proc_num && cacheArray[i]->findLine(address) != NULL && cacheArray[i]->findLine(address)->getFlags() != INVALID)
					{
					  	if ((cacheArray[i]->findLine(address))->getFlags() == MODIFIED)
						{
						    cacheArray[i]->num_of_interventions++;
						    cacheArray[i]->num_of_flushes++;
						    cacheArray[i]->writeBack(address); // write back
					    }
					    if ((cacheArray[i]->findLine(address))->getFlags() == EXCLUSIVE)
					    {
						    cacheArray[i]->num_of_interventions++;
						    //cacheArray[i]->writeBack(address); 
					    }

					    if((cacheArray[i]->findLine(address))->getFlags() == SHARED_MODIFIED)
					    {
					    	cacheArray[i]->num_of_flushes++;
					    	cacheArray[i]->writeBack(address); // write back
					    }
					    
					    (cacheArray[i]->findLine(address))->setFlags(SHARED_CLEAN);
					}
			    }
			}
			else
			{
                (cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			}
            return;
		}
	}
    
    // for EXCLUSIVE state
    if ((cacheArray[proc_num]->findLine(address))->getFlags() == EXCLUSIVE)
	{	//PrRd --- For Read : EXCLUSIVE ---> EXCLUSIVE | FOR Write : EXCLUSIVE ---> MODIFIED
		if(op=='r')
		{	
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(EXCLUSIVE);
			return;
		}
		else
		{	//EXCLUSIVE ---> MODIFIED
			cacheArray[proc_num]->Access(address, op);
			(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
			return;
		}
	}

	// for MODIFIED state, no state change and no bus transaction
	if ((cacheArray[proc_num]->findLine(address))->getFlags() == MODIFIED)
	{
		cacheArray[proc_num]->Access(address, op);
		(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
		
		return;
	}

	// for SHARED_CLEAN state

	if ((cacheArray[proc_num]->findLine(address))->getFlags() == SHARED_CLEAN)
	{
        if(op=='r')
        {
        	cacheArray[proc_num]->Access(address, op);
        	(cacheArray[proc_num]->findLine(address))->setFlags(SHARED_CLEAN);
        }
        else
        {
        	cacheArray[proc_num]->Access(address, op);
        	//cacheArray[proc_num]->num_of_flushes++;
        	if(C==true)
        	{
        		(cacheArray[proc_num]->findLine(address))->setFlags(SHARED_MODIFIED);
        	}
        	else
        	{
        		(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
        	}
        	
        	for (int i = 0; i<num_proc; i++)
		    {
			    if (i != proc_num && cacheArray[i]->findLine(address) != NULL)
				{
					// change SHARED_MODIFIED state to SHARED_CLEAN if any
					if((cacheArray[i]->findLine(address))->getFlags() == SHARED_MODIFIED)
                    {
                        (cacheArray[i]->findLine(address))->setFlags(SHARED_CLEAN);
                        //cacheArray[proc_num]->num_of_flushes++;
					    //cacheArray[i]->writeBack(address); // write backs
                    }
                }
            }
        } 
        return;
	}

	// for SHARED_MODIFIED state
	if ((cacheArray[proc_num]->findLine(address))->getFlags() == SHARED_MODIFIED)
	{
		if(op=='r')
		{
			cacheArray[proc_num]->Access(address, op);
            (cacheArray[proc_num]->findLine(address))->setFlags(SHARED_MODIFIED);
		}
		else
		{
			cacheArray[proc_num]->Access(address, op);
            if(C==true)
            {
            	// when write to it and there are other copies, stay the state unchanged
            	(cacheArray[proc_num]->findLine(address))->setFlags(SHARED_MODIFIED);
            }
            else
            {
            	// otherwise, change to MODIFIED
            	(cacheArray[proc_num]->findLine(address))->setFlags(MODIFIED);
            }
            //cacheArray[proc_num]->writeBack(address); // write back
		}
	}
	return;
}



int main(int argc, char *argv[])
{
	
	ifstream fin;
	FILE * pFile;

    int pro;
    uchar op;
    uint addr;

	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	/*****uncomment the next five lines****/
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/
	char *fname =  (char *)malloc(20);
 	fname = argv[6];
    vector <Cache*> cacheArray;
	
	for (int i = 0; i< num_processors; i++)
	{
		Cache* c = new Cache(cache_size, cache_assoc, blk_size);
		cacheArray.push_back(c);
	}
	//****************************************************//
	//**printf("===== Simulator configuration =====\n");**//
	//*******print out simulator configuration here*******//
	//****************************************************//

 
	//*********************************************//
        //*****create an array of caches here**********//
	//*********************************************//	

	pFile = fopen(fname,"r");
	
	if(pFile == 0)
	{   
		printf("Trace file problem\n");
		exit(0);
	}

    if(protocol==0)
    {
    	goto MSI;
    }

    if(protocol==1)
    {
    	goto MESI;
    }

    if(protocol==2)
    {
    	goto DRAGON;
    }

    MSI:
	while(!feof(pFile))
	{
		fscanf(pFile,"%d %c %x\n",&pro,&op,&addr);
        MSI(cacheArray, pro, addr, op, num_processors);
	}
	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
	//*****propagate each request down through memory hierarchy**********//
	//*****by calling cacheArray[processor#]->Access(...)***************//
	///******************************************************************//
	fclose(pFile);

	//********************************//
	//print out all caches' statistics //
	//********************************//

	goto RESULT;

	MESI:
	while(!feof(pFile))
	{
		fscanf(pFile,"%d %c %x\n",&pro,&op,&addr);
        MESI(cacheArray, pro, addr, op, num_processors);
	}

	fclose(pFile);

    goto RESULT1;

    DRAGON:
    while(!feof(pFile))
	{
		fscanf(pFile,"%d %c %x\n",&pro,&op,&addr);
        DRAGON(cacheArray, pro, addr, op, num_processors);
	}

	fclose(pFile);

    goto RESULT2;


	RESULT: // give the final output of MSI
	for (int i = 0; i<num_processors; i++)
	{
		cout << "============ Simulation results (Cache " << i << ") ============" << endl;
		cout << "01. number of reads:                " << cacheArray[i]->getReads() << endl;
		cout << "02. number of read misses:          " << cacheArray[i]->getRM() << endl;
		cout << "03. number of writes:               " << cacheArray[i]->getWrites() << endl;
		cout << "04. number of write misses:         " << cacheArray[i]->getWM() << endl;
		cout << "05. total miss rate:                " << fixed << setprecision(2)<< (cacheArray[i]->getWM()+cacheArray[i]->getRM()) * 100.0 / (cacheArray[i]->getReads()+cacheArray[i]->getWrites())<< '%' << endl;
		cout << "06. number of write backs:          " << cacheArray[i]->getWB() << endl;
		cout << "07. number of cache to cache transfers:       " << cacheArray[i]->num_of_cache_to_cache_transfer << endl;
		cout << "08. number of memory transactions:      " << cacheArray[i]->num_of_mem_trans+cacheArray[i]->getRM()+cacheArray[i]->getWM()+cacheArray[i]->getWB()<< endl;
		cout << "09. number of interventions:            " << cacheArray[i]->num_of_interventions << endl;
		cout << "10. number of invalidations:            " << cacheArray[i]->num_of_invalidations << endl;
		cout << "11. number of flushes:              " << cacheArray[i]->num_of_flushes << endl;
	}
    return 0;

	RESULT1: // give the final result of MESI 
	for (int i = 0; i<num_processors; i++)
	{
		cout << "============ Simulation results (Cache " << i << ") ============" << endl;
		cout << "01. number of reads:                " << cacheArray[i]->getReads() << endl;
		cout << "02. number of read misses:          " << cacheArray[i]->getRM() << endl;
		cout << "03. number of writes:               " << cacheArray[i]->getWrites() << endl;
		cout << "04. number of write misses:         " << cacheArray[i]->getWM() << endl;
		cout << "05. total miss rate:                " << fixed << setprecision(2)<< (cacheArray[i]->getWM()+cacheArray[i]->getRM()) * 100.0 / (cacheArray[i]->getReads()+cacheArray[i]->getWrites())<< '%' << endl;
		cout << "06. number of write backs:          " << cacheArray[i]->getWB() << endl;
		cout << "07. number of cache to cache transfers:      " << cacheArray[i]->num_of_cache_to_cache_transfer << endl;
		cout << "08. number of memory transactions:      " << cacheArray[i]->getWM() + cacheArray[i]->getRM() + cacheArray[i]->getWB() - cacheArray[i]->num_of_cache_to_cache_transfer << endl;
		cout << "09. number of interventions:            " << cacheArray[i]->num_of_interventions << endl;
		cout << "10. number of invalidations:            " << cacheArray[i]->num_of_invalidations << endl;
		cout << "11. number of flushes:              " << cacheArray[i]->num_of_flushes << endl;
	}
	return 0;

	RESULT2: // give the final result of DRAGON
	for (int i = 0; i<num_processors; i++)
	{
		cout << "============ Simulation results (Cache " << i << ") ============" << endl;
		cout << "01. number of reads:                " << cacheArray[i]->getReads() << endl;
		cout << "02. number of read misses:          " << cacheArray[i]->getRM() << endl;
		cout << "03. number of writes:               " << cacheArray[i]->getWrites() << endl;
		cout << "04. number of write misses:         " << cacheArray[i]->getWM() << endl;
		cout << "05. total miss rate:                " << fixed << setprecision(2)<< (cacheArray[i]->getWM()+cacheArray[i]->getRM()) * 100.0 / (cacheArray[i]->getReads()+cacheArray[i]->getWrites())<< '%' << endl;
		cout << "06. number of write backs:          " << cacheArray[i]->getWB() << endl;
		cout << "07. number of cache to cache transfers:     " << cacheArray[i]->num_of_cache_to_cache_transfer << endl;
		cout << "08. number of memory transactions:      " << cacheArray[i]->getWM() + cacheArray[i]->getRM() + cacheArray[i]->getWB()+cacheArray[i]->num_of_flushes<< endl;
		cout << "09. number of interventions:            " << cacheArray[i]->num_of_interventions << endl;
		cout << "10. number of invalidations:            " << cacheArray[i]->num_of_invalidations << endl;
		cout << "11. number of flushes:              " << cacheArray[i]->num_of_flushes << endl;
	}
	return 0;
	
}
