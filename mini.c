#ifdef MPI_BUILD_PROFILING
#undef MPI_BUILD_PROFILING
#endif
#include <stdio.h>
#include <papi.h>
#include "mpi.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#if defined(NEEDS_STDLIB_PROTOTYPES)
#include "protofix.h"
#endif

struct stat st = {0};
int MINI_Trace_hasBeenInit = 0;
int MINI_Trace_hasBeenFinished = 0;     
int global;

FILE *fp;
float real_time, proc_time;
long long ins1, ins2,t1,t2;
int EventSet=PAPI_NULL,Events[30],i_mode=0,en_time=0;
int num_hwcntrs=0,bcount=0,imod=0,glob_size=0,glob_np=0,buff=250;
long long values[1],start_time,end_time,elapsed_time;
char longmsg[120000],temp_buf[100],temp_long[1000];
char *testt;
#define TRACE_PRINTF(msg) \
if ( (MINI_Trace_hasBeenInit) && (!MINI_Trace_hasBeenFinished) ) {\
  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank ); \
  printf( "%s\n", msg ); \
  fflush( stdout ); \
}


int encode_datatype(const char *dat) {
	int res=0;
       if(strcmp("MPI_DOUBLE_PRECISION",dat)==0 || strcmp("MPI_DOUBLE",dat)==0) {
           res=0;
        }

       if(strcmp("MPI_INTEGER",dat)==0 || strcmp("MPI_INT",dat)==0) {
           res=1;
        }

       if(strcmp("MPI_CHARACTER",dat)==0 || strcmp("MPI_CHAR",dat)==0) {
           res=2;
        }

       if(strcmp("MPI_SHORT",dat)==0) {
           res=3;
        }

       if(strcmp("MPI_LONG",dat)==0) {
           res=4;
        }
       if(strcmp("MPI_REAL",dat)==0 || strcmp("MPI_FLOAT",dat)==0) {
           res=5;
        }

       if(strcmp("MPI_BYTE",dat)==0) {
           res=6;
        }

     
	return res;
}



int   MPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm )
const void * sendbuf;
void * recvbuf;
int count;
MPI_Datatype datatype;
MPI_Op op;
MPI_Comm comm;
{
  int   returnVal;
  int llrank;
  char msg[100];
  int np;
  char nam[MPI_MAX_OBJECT_NAME];
  if(en_time==1) end_time=PAPI_get_real_usec();

  MPI_Type_get_name(datatype,nam,&np);

  np=encode_datatype((const char*)&nam);


 if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");


  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );

  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  ins2=values[0];
  strcat(longmsg,msg);

  sprintf(msg, "%d comm_size %d\n",llrank,global);
  strcat(longmsg,msg);


  bcount=count+2;

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");
  ins2=values[0];

  returnVal = PMPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm );

  PAPI_accum_counters(values, 1);

  ins1=values[0];
  if(np>0) sprintf(msg, "%d allReduce %d %lld %d\n",llrank,count,ins1-ins2,np);
  else sprintf(msg, "%d allReduce %d %lld\n",llrank,count,ins1-ins2);
  strcat(longmsg,msg);

 PAPI_accum_counters(values, 1);
 
  ins1=values[0];
  if(en_time==1) start_time=PAPI_get_real_usec();

  return returnVal;
}

int  MPI_Gather( sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, root, comm )
const void * sendbuf;
int sendcount;
MPI_Datatype sendtype;
void * recvbuf;
int recvcnt;
MPI_Datatype recvtype;
int root;
MPI_Comm comm;
{
  int  returnVal;
  int llrank;
  int np,np2;
  char msg[300];
  char nam[MPI_MAX_OBJECT_NAME];
  if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );

  if(en_time==1) {
        end_time=PAPI_get_real_usec();
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);
 
  ins2=values[0];
  MPI_Type_get_name(sendtype,nam,&np);
  np=encode_datatype((const char*)&nam);

  MPI_Type_get_name(recvtype,nam,&np2);
  np2=encode_datatype((const char*)&nam);

  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  strcat(longmsg,msg);

   sprintf(msg, "%d gather %d %d %d ",
              llrank,sendcount, recvcnt,root);
  strcat(longmsg,msg);

  if(np>0 || np2>0) sprintf(msg, "%d %d\n",np,np2);
  else sprintf(msg, "\n");

  strcat(longmsg,msg);
  
  returnVal = PMPI_Gather( sendbuf, sendcount, sendtype, recvbuf, recvcnt,
                             recvtype, root,comm );

  bcount=bcount+2;

  if(en_time==1) start_time=PAPI_get_real_usec();
  PAPI_accum_counters(values, 1);
  ins1=values[0];

  return returnVal;
}



int  MPI_Alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm )
const void * sendbuf;
int sendcount;
MPI_Datatype sendtype;
void * recvbuf;
int recvcnt;
MPI_Datatype recvtype;
MPI_Comm comm;
{
  int  returnVal;
  int llrank;
  int np,np2;
  char msg[300];
  char nam[MPI_MAX_OBJECT_NAME];

  if(en_time==1) end_time=PAPI_get_real_usec();
  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  MPI_Type_get_name(sendtype,nam,&np);
  np=encode_datatype((const char*)&nam);
  MPI_Type_get_name(recvtype,nam,&np2);
  np2=encode_datatype((const char*)&nam);

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );

  if(en_time==1) {
        end_time=PAPI_get_real_usec();
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  ins2=values[0];
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  strcat(longmsg,msg);
  sprintf(msg, "%d allToAll %d %d ",
              llrank,sendcount, recvcnt);
  strcat(longmsg,msg);

  if(np>0 || np2>0) {
  	sprintf(msg, "%d %d\n",np,np2);
  }
  else sprintf(msg, "\n");
  strcat(longmsg,msg);

  returnVal = PMPI_Alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcnt, 
			     recvtype, comm );
  if(en_time==1) start_time=PAPI_get_real_usec();
  PAPI_accum_counters(values, 1);
  ins1=values[0];
  
  return returnVal;
}

int   MPI_Gatherv( sendbuf, sendcnts, sendtype, recvbuf, recvcnts, displs, recvtype, root, comm )
const void * sendbuf;
int sendcnts;
MPI_Datatype sendtype;
void * recvbuf;
const int * recvcnts;
const int * displs;
MPI_Datatype recvtype;
int root;
MPI_Comm comm;
{
  int   returnVal;
  int llrank;
  char msg[300];
  int i,max_recv=0,max_recv_displ=0,s_buffer=0;
  int np,np2;
  char nam[MPI_MAX_OBJECT_NAME];

  if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  MPI_Type_get_name(sendtype,nam,&np);
  np=encode_datatype((const char*)&nam);
  MPI_Type_get_name(recvtype,nam,&np2);
  np2=encode_datatype((const char*)&nam);

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  ins2=values[0];
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);


  strcat(longmsg,msg);

  for(i=0;i<global;i++) {
    if (recvcnts[i]>max_recv) max_recv=recvcnts[i];
    if (displs[i]>max_recv_displ) max_recv_displ=displs[i];
  }
  sprintf(msg, "%d GatherV %d ",
           llrank,s_buffer );

  strcat(longmsg,msg);

  for (i=0;i<global;i++) {
   sprintf(msg,"%d ",recvcnts[i]);
   strcat(longmsg,msg);
  }
  for (i=0;i<global;i++) {

   sprintf(msg,"%d ",displs[i]);
   strcat(longmsg,msg);

  }

  if(np>0 || np2>0) {
        sprintf(msg,"%d %d\n",np,np2);
  }
  else strcat(longmsg,"\n");
  strcat(longmsg,msg);

  bcount=bcount+2;

  returnVal = PMPI_Gatherv( sendbuf, sendcnts, sendtype, recvbuf, recvcnts,
                            displs, recvtype, root, comm );

  if(en_time==1) start_time=PAPI_get_real_usec();

  PAPI_accum_counters(values, 1);
  ins1=values[0];
  return returnVal;
}

int   MPI_Allgather( sendbuf, sendcnts, sendtype, recvbuf, recvcnts, recvtype, comm )
const void * sendbuf;
int sendcnts;
MPI_Datatype sendtype;
void * recvbuf;
int recvcnts;
MPI_Datatype recvtype;
MPI_Comm comm;
{
  int   returnVal;
  int llrank;
  char msg[300];
  int i;
  int np,np2;
  char nam[MPI_MAX_OBJECT_NAME];

  if(en_time==1) end_time=PAPI_get_real_usec();
  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  MPI_Type_get_name(sendtype,nam,&np);
  np=encode_datatype((const char*)&nam);
  MPI_Type_get_name(recvtype,nam,&np2);
  np2=encode_datatype((const char*)&nam);


  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  ins2=values[0];
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  strcat(longmsg,msg);

  sprintf(msg, "%d allGather %d %d",
           llrank,sendcnts, recvcnts );

  strcat(longmsg,msg);

  if(np>0 || np2>0) {
	  sprintf(msg,"%d %d\n",np,np2);
  }
  else sprintf(msg,"\n");
  strcat(longmsg,msg);

  bcount=bcount+2;

  returnVal = PMPI_Allgather( sendbuf, sendcnts, sendtype, recvbuf,
                               recvcnts, recvtype, comm );

  if(en_time==1) start_time=PAPI_get_real_usec();

  PAPI_accum_counters(values, 1);
  ins1=values[0];

  return returnVal;
}


int   MPI_Allgatherv( sendbuf, sendcnts, sendtype, recvbuf, recvcnts, displs, recvtype, comm )
const void * sendbuf;
int sendcnts;
MPI_Datatype sendtype;
void * recvbuf;
const int * recvcnts;
const int * displs;
MPI_Datatype recvtype;
MPI_Comm comm;
{
  int   returnVal;
  int llrank;
  char msg[300];
  int i,max_send=0,max_send_displ=0,max_recv=0,max_recv_displ=0,s_buffer=0,r_buffer=0;
  int np,np2;
  char nam[MPI_MAX_OBJECT_NAME];

  if(en_time==1) end_time=PAPI_get_real_usec();
  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  MPI_Type_get_name(sendtype,nam,&np);
  np=encode_datatype((const char*)&nam);
  MPI_Type_get_name(recvtype,nam,&np2);
  np2=encode_datatype((const char*)&nam);


  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  ins2=values[0];
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  strcat(longmsg,msg);

  for(i=0;i<global;i++) {
    if (recvcnts[i]>max_recv) max_recv=recvcnts[i];
    if (displs[i]>max_send_displ) max_send_displ=displs[i];
  }
  s_buffer=max_send+max_send_displ;
  r_buffer=max_recv+max_recv_displ;
  sprintf(msg, "%d allGatherV %d ",
           llrank,sendcnts );

  strcat(longmsg,msg);

  for (i=0;i<global;i++) {
   sprintf(msg,"%d ",recvcnts[i]);
   strcat(longmsg,msg);
  }
  for (i=0;i<global;i++) {

   sprintf(msg,"%d ",displs[i]);
   strcat(longmsg,msg);

  }


  if(np>0 || np2>0) {
	  sprintf(msg,"%d %d\n",np,np2);
  }
  else sprintf(msg,"\n");
  strcat(longmsg,msg);

  bcount=bcount+2;

  returnVal = PMPI_Allgatherv( sendbuf, sendcnts, sendtype, recvbuf,
                               recvcnts, displs, recvtype, comm );

  if(en_time==1) start_time=PAPI_get_real_usec();

  PAPI_accum_counters(values, 1);
  ins1=values[0];

  return returnVal;
}


int   MPI_Reduce_scatter( sendbuf, recvbuf, recvcnts, datatype, op, comm )
const void * sendbuf;
void * recvbuf;
const int * recvcnts;
MPI_Datatype datatype;
MPI_Op op;
MPI_Comm comm;
{
  int   returnVal;
  int llrank;
  char msg[300];
  int i;
  int np;
  char nam[MPI_MAX_OBJECT_NAME];
  if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  MPI_Type_get_name(datatype,nam,&np);
  np=encode_datatype((const char*)&nam);


  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  ins2=values[0];
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  strcat(longmsg,msg);
  sprintf(msg, "%d reduceScatter ",
           llrank );

  strcat(longmsg,msg);

  for (i=0;i<global;i++) {
   sprintf(msg,"%d ",recvcnts[i]);
   strcat(longmsg,msg);
  }

  bcount=bcount+2;

  PAPI_accum_counters(values, 1);
  ins2=values[0];

  returnVal = PMPI_Reduce_scatter( sendbuf, recvbuf, recvcnts, datatype, op,
                                   comm );

  PAPI_accum_counters(values, 1);
  ins1=values[0];

  sprintf(msg,"%lld ",ins1-ins2);
  strcat(longmsg,msg);


  if(np>0) sprintf(msg,"%d\n",np);
  else sprintf(msg,"\n");
  strcat(longmsg,msg);

  if(en_time==1) start_time=PAPI_get_real_usec();

  PAPI_accum_counters(values, 1);
  ins1=values[0];

  return returnVal;
}

int   MPI_Alltoallv( sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm )
const void * sendbuf;
const int * sendcnts;
const int * sdispls;
MPI_Datatype sendtype;
void * recvbuf;
const int * recvcnts;
const int * rdispls;
MPI_Datatype recvtype;
MPI_Comm comm;
{
  int   returnVal;
  int llrank;
  char msg[300];
  int i,max_send=0,max_send_displ=0,max_recv=0,max_recv_displ=0,s_buffer=0,r_buffer=0;
  int np,np2;
  char nam[MPI_MAX_OBJECT_NAME];

/*
    MPI_Alltoallv - prototyping replacement for MPI_Alltoallv
    Trace the beginning and ending of MPI_Alltoallv.
*/
 if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  MPI_Type_get_name(sendtype,nam,&np);
  np=encode_datatype((const char*)&nam);
  MPI_Type_get_name(recvtype,nam,&np2);
  np2=encode_datatype((const char*)&nam);


  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  ins2=values[0];
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }
  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  strcat(longmsg,msg);

  for(i=0;i<global;i++) {
    if (sendcnts[i]>max_send) max_send=sendcnts[i];
    if (recvcnts[i]>max_recv) max_recv=recvcnts[i];
    if (sdispls[i]>max_send_displ) max_send_displ=sdispls[i];
    if (rdispls[i]>max_recv_displ) max_recv_displ=rdispls[i];
  }
  s_buffer=max_send+max_send_displ;
  r_buffer=max_recv+max_recv_displ;
  sprintf(msg, "%d allToAllV %d ",
           llrank,s_buffer );

  strcat(longmsg,msg);
//   printf("%d ",llrank);
   
  for (i=0;i<global;i++) {
   sprintf(msg,"%d ",sendcnts[i]);
   strcat(longmsg,msg);
  }

  sprintf(msg, "%d ",r_buffer );
  strcat(longmsg,msg);

  for (i=0;i<global;i++) {
   sprintf(msg,"%d ",recvcnts[i]);
   strcat(longmsg,msg);
  }

  if(np>0 || np2>0) {
  	sprintf(msg,"%d %d\n",np,np2);
  }
  else sprintf(msg,"\n");
  strcat(longmsg,msg);

  bcount=bcount+2;
  
  returnVal = PMPI_Alltoallv( sendbuf, sendcnts, sdispls, sendtype, recvbuf, 
			      recvcnts, rdispls, recvtype, comm );
  if(en_time==1) start_time=PAPI_get_real_usec();

  PAPI_accum_counters(values, 1);
  ins1=values[0];

  return returnVal;
}

int   MPI_Barrier( comm )
MPI_Comm comm;
{
  int   returnVal;
  int llrank;
  char msg[100];

  if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");
  ins2=values[0];
  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);


  strcat(longmsg,msg);

  sprintf(msg,"%d barrier\n",llrank);
  strcat(longmsg,msg);

  bcount++;
  
  returnVal = PMPI_Barrier( comm );

  PAPI_accum_counters(values, 1);
  ins1=values[0];
  if(en_time==1) start_time=PAPI_get_real_usec();
  return returnVal;
}

int   MPI_Bcast( buffer, count, datatype, root, comm )
void * buffer;
int count;
MPI_Datatype datatype;
int root;
MPI_Comm comm;
{
  int   returnVal;
  int llrank;
  char msg[100];
  int np;
  char nam[MPI_MAX_OBJECT_NAME];
  MPI_Type_get_name(datatype,nam,&np);
  np=encode_datatype((const char*)&nam);

  if(en_time==1) end_time=PAPI_get_real_usec();
  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");
  ins2=values[0];
  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }
  msg[0]='\0';
  if(en_time==1) sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  else  sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);
  strcat(longmsg,msg);
  sprintf(msg, "%d comm_size %d\n",llrank,global);
  strcat(longmsg,msg);
  if(root>0 || np>0) sprintf(msg, "%d bcast %d %d %d\n",llrank,count,root,np);
  else sprintf(msg, "%d bcast %d\n",llrank,count);
  strcat(longmsg,msg);
  bcount=bcount+3;
  returnVal = PMPI_Bcast( buffer, count, datatype, root, comm );
  PAPI_accum_counters(values, 1);
  ins1=values[0];
  if(en_time==1) start_time=PAPI_get_real_usec();
  return returnVal;
}

int   MPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm )
const void * sendbuf;
void * recvbuf;
int count;
MPI_Datatype datatype;
MPI_Op op;
int root;
MPI_Comm comm;
{
  int   returnVal;
  int llrank;
  char msg[100];
  int np;
  char nam[MPI_MAX_OBJECT_NAME];

  if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");
  ins2=values[0];
  MPI_Type_get_name(datatype,nam,&np);
  np=encode_datatype((const char*)&nam);

  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );

  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);
  sprintf(msg, "%d comm_size %d\n",llrank,global);

  PAPI_accum_counters(values, 1);
  ins2=values[0];

  returnVal = PMPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm );

  bcount=bcount+3;
  PAPI_accum_counters(values, 1);
  if(en_time==1) start_time=PAPI_get_real_usec();
  ins1=values[0];
  if(root>0 || np >0) sprintf(msg, "%d reduce %d %lld %d %d",llrank,count,ins1-ins2,root,np);
  else sprintf(msg, "%d reduce %d %lld",llrank,count,ins1-ins2);
  PAPI_accum_counters(values, 1);
  ins1=values[0];
  return returnVal;
}

int   MPI_Comm_rank( comm, rank )
MPI_Comm comm;
int * rank;
{
  int   returnVal;
  int llrank;
  char msg[100];

  if(en_time==1) end_time=PAPI_get_real_usec();
  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");
  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );

  ins2=values[0];
  if(en_time==1) {
	 sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);
  strcat(longmsg,msg);

  returnVal = PMPI_Comm_rank( comm, rank );

  PAPI_accum_counters(values, 1);
  ins1=values[0];
  if(en_time==1) start_time=PAPI_get_real_usec();

  return returnVal;
}

int   MPI_Comm_size( comm, size )
MPI_Comm comm;
int * size;
{
  int   returnVal;
  int llrank;
  char msg[100];

 if(en_time==1) end_time=PAPI_get_real_usec();
  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );

  ins2=values[0];
  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  strcat(longmsg,msg);

  returnVal = PMPI_Comm_size( comm, size );
  global=*size;
  PAPI_accum_counters(values, 1);
  ins1=values[0];
  if(en_time==1) start_time=PAPI_get_real_usec();

  return returnVal;
}

int  MPI_Finalize(  )
{
  int  returnVal;
  int llrank;

 if(en_time==1) {
	end_time=PAPI_get_real_usec();
        printf("total time %lld\n",end_time-start_time); 
 }
  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );

  printf("%d total %lld\n",llrank,values[0]-t1);
  if (bcount>0 )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }
  if(en_time==1) fprintf(fp,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  else  fprintf(fp,"%d compute %lld\n",llrank,values[0]-ins1);
  fprintf(fp,"%d finalize",llrank);
  fclose(fp);

  PAPI_stop_counters(values, 1);

  MINI_Trace_hasBeenFinished = 1;  
  returnVal = PMPI_Finalize(  );
  return returnVal;
}

int  MPI_Init( argc, argv )
int * argc;
char *** argv;
{
  int  returnVal;
  int llrank,event_code;
  char file[50];
  returnVal = PMPI_Init( argc, argv );
  testt=getenv("MINI_TIME");
  en_time=atoi(testt);

  testt=getenv("MINI_METRIC");
  PAPI_library_init(PAPI_VER_CURRENT);
  if(PAPI_create_eventset(&EventSet)!=PAPI_OK) {
        printf("Could not create the EventSet");
  }
  PAPI_event_name_to_code(testt,&event_code);

  PAPI_add_event(EventSet, event_code);
  Events[0]=event_code;
  MINI_Trace_hasBeenInit = 1;


  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  
  if (stat("$PWD/ti_traces", &st) == -1) {
    mkdir("$PWD/ti_traces", 0700);
  }  
  sprintf(file,"ti_traces/ti_trace%d.txt",llrank);
  fp=fopen(file, "wb");
  fprintf(fp,"%d init 1\n",llrank);

  if(PAPI_start_counters(Events,1)!=PAPI_OK) {
        printf("error1\n");
  }
  if(en_time==1) start_time=PAPI_get_real_usec();
  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  if(en_time==1) start_time=PAPI_get_real_usec();
  ins1=values[0];
  t1=values[0];

  return returnVal;
}

int  MPI_Init_thread( argc, argv,requir,provided )
int * argc;
char *** argv;
int requir;
int *provided;
{
  int  returnVal;
  int llrank,event_code;
  char file[50];
  char cwd[1024];
  returnVal = PMPI_Init( argc, argv );
  testt=getenv("MINI_TIME");
  en_time=atoi(testt);

  testt=getenv("MINI_METRIC");
  PAPI_library_init(PAPI_VER_CURRENT);
  if(PAPI_create_eventset(&EventSet)!=PAPI_OK) {
	printf("Could not create the EventSet");
  }
  PAPI_event_name_to_code(testt,&event_code);

  PAPI_add_event(EventSet, event_code);

  Events[0]=event_code;

  MINI_Trace_hasBeenInit = 1;

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  getcwd(cwd,1024);
  strcat(cwd,"/ti_traces");
  printf("\n\n%s\n\n",cwd);
  if (stat(cwd, &st) == -1) {
    mkdir(cwd, 0700);
  }  
  sprintf(file,"ti_traces/ti_trace%d.txt",llrank);
  fp=fopen(file, "wb");
  fprintf(fp,"%d init 1\n",llrank);

  if(PAPI_start_counters(Events,1)!=PAPI_OK) printf("Can not start the PAPI counter\n");
  
  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  if(en_time==1) start_time=PAPI_get_real_usec();
  ins1=values[0];
  t1=values[0];


  return returnVal;
}

int  MPI_Initialized( flag )
int * flag;
{
  int  returnVal;
  
  returnVal = PMPI_Initialized( flag );

  return returnVal;
}

#ifdef FOO
/* Don't trace the timer calls */
double  MPI_Wtick(  )
{
  double  returnVal;
  int llrank;

  returnVal = PMPI_Wtick(  );

  return returnVal;
}

double  MPI_Wtime(  )
{
  double  returnVal;
  int llrank;

  returnVal = PMPI_Wtime(  );

  return returnVal;
}
#endif

int  MPI_Irecv( buf, count, datatype, source, tag, comm, request )
void * buf;
int count;
MPI_Datatype datatype;
int source;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  int  llrank;
  char msg[100];
  int np;
  char nam[MPI_MAX_OBJECT_NAME];
  MPI_Type_get_name(datatype,nam,&np);

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");
  if(en_time==1) end_time=PAPI_get_real_usec();
  np=encode_datatype((const char*)&nam);

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }
  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  strcat(longmsg,msg);

  bcount=bcount+2;
  glob_size=count;
  glob_np=np;

  returnVal = PMPI_Irecv( buf, count, datatype, source, tag, comm, request );

  PAPI_accum_counters(values, 1);
  ins1=values[0];
  i_mode=1;
  if(en_time==1) start_time=PAPI_get_real_usec();
  return returnVal;
}

int  MPI_Isend( buf, count, datatype, dest, tag, comm, request )
const void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
MPI_Request * request;
{
  int  returnVal;
  int  llrank;
  char msg[100];
  int np;
  char nam[MPI_MAX_OBJECT_NAME];
  if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");

  MPI_Type_get_name(datatype,nam,&np);
  np=encode_datatype((const char*)&nam);

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  ins2=values[0];
  if (bcount>buff )
   {
       fprintf(fp, longmsg);
       longmsg[0]='\0';
        bcount=0;
   }

  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  strcat(longmsg,msg);

  if(np>0) sprintf(msg, "%d Isend %d %d %d",llrank,dest,count,np);
  else sprintf(msg, "%d Isend %d %d",llrank,dest,count);
  strcat(longmsg,msg);

  returnVal = PMPI_Isend( buf, count, datatype, dest, tag, comm, request );

  PAPI_accum_counters(values, 1);
  ins1=values[0];
  if(en_time==1) start_time=PAPI_get_real_usec();

  return returnVal;
}

int  MPI_Recv( buf, count, datatype, source, tag, comm, status )
void * buf;
int count;
MPI_Datatype datatype;
int source;
int tag;
MPI_Comm comm;
MPI_Status * status;
{
  int  returnVal;
  int  llrank;
  char msg[100];
  int np;
  char nam[MPI_MAX_OBJECT_NAME];
  MPI_Type_get_name(datatype,nam,&np);

  if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");
  np=encode_datatype((const char*)&nam);

#ifdef HAVE_MPI_STATUS_IGNORE
  MPI_Status    tmp_status;
  if (status == MPI_STATUS_IGNORE)
      status = &tmp_status;
#endif
  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );
  ins2=values[0];

  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }

  if(en_time==1) {
         sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
  }
  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

  strcat(longmsg,msg);

  if(np>0)  sprintf(msg, "%d recv %d %d %d\n",
             llrank,source,count,np );
  else sprintf(msg, "%d recv %d %d\n",
           llrank,source,count);
  strcat(longmsg,msg);
  returnVal = PMPI_Recv( buf, count, datatype, source, tag, comm, status );

 bcount=bcount+2;
 PAPI_accum_counters(values, 1);
 ins1=values[0];
 if(en_time==1) start_time=PAPI_get_real_usec();
  
  return returnVal;
}

int  MPI_Send( buf, count, datatype, dest, tag, comm )
const void * buf;
int count;
MPI_Datatype datatype;
int dest;
int tag;
MPI_Comm comm;
{
  int  returnVal;
  int  llrank;
  char msg[100],temp_buff[100];
  int np;
  char nam[MPI_MAX_OBJECT_NAME];
  MPI_Type_get_name(datatype,nam,&np);

 if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");
  np=encode_datatype((const char*)&nam);

  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );  
  ins2=values[0];
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }
  if(i_mode==0) {

	  if(en_time==1) {
         	sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
          }
	  else sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);

	strcat(longmsg,msg);
	if(np>0) sprintf(msg, "%d send %d %d %d\n",
             llrank,dest,count,np);
  	else sprintf(msg, "%d send %d %d\n",
           llrank,dest,count);
         strcat(longmsg,msg);

   }
   else  {
	  if(en_time==1) {
         	sprintf(temp_buff,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);
          }
  	else sprintf(temp_buff,"%d compute %lld\n",llrank,values[0]-ins1);

	strcat(temp_long,temp_buff);
  
         if(np>0) sprintf(temp_buff, "%d send %d %d %d\n",
             llrank,dest,count,np);
         else sprintf(temp_buff, "%d send %d %d\n",
           llrank,dest,count);

         strcat(temp_long,temp_buff);
         temp_buff[0]='\0';
  }
  
  returnVal = PMPI_Send( buf, count, datatype, dest, tag, comm );


  bcount=bcount+2;
  PAPI_accum_counters(values, 1);
  ins1=values[0];
  if(en_time==1) start_time=PAPI_get_real_usec();

  return returnVal;
}

int   MPI_Wait( request, status )
MPI_Request * request;
MPI_Status * status;
{
  int   returnVal;
  int llrank;
  char msg[100];

  if(en_time==1) end_time=PAPI_get_real_usec();

  if(PAPI_accum_counters(values, 1)!=PAPI_OK) printf("This PAPI event is not supported\n");
  PMPI_Comm_rank( MPI_COMM_WORLD, &llrank );

  ins2=values[0];
  if (bcount>buff )
  {
   fprintf(fp, longmsg);
   longmsg[0]='\0';
   bcount=0;
  }
  returnVal = PMPI_Wait( request, status );
  if(i_mode==1) {
    if(glob_np>0) sprintf(msg,"%d Irecv %d %d %d\n",llrank,status->MPI_SOURCE,glob_size,glob_np);
    else sprintf(msg,"%d Irecv %d %d\n",llrank,status->MPI_SOURCE,glob_size);
    strcat(longmsg,msg);
    strcat(longmsg,temp_long);
  }
  if(en_time==1) sprintf(msg,"%d compute %lld %.6f\n",llrank,values[0]-ins1,(double)(end_time-start_time)/1000000);

  else  sprintf(msg,"%d compute %lld\n",llrank,values[0]-ins1);
  strcat(longmsg,msg);
  
  sprintf(msg,"%d wait\n",llrank);
  strcat(longmsg,msg);
  i_mode=0;
  temp_long[0]='\0';

  bcount=bcount+3;
  PAPI_accum_counters(values, 1);
  ins1=values[0];
  if(en_time==1) start_time=PAPI_get_real_usec();
  return returnVal;
}
