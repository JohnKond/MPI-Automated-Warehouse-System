#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h> 
#include <stddef.h> 

#define INFO 0
#define BUFFER_SIZE 200

int NUM_SERVERS;


#define SERVER_TAG 0
#define PROBE_TAG 1
#define REPLY_TAG 2
#define CLIENT_TAG 3
#define CONNECT_TAG 4
#define ELECTION_TAG 5
#define PRINT_TAG 6
#define SUPPLY_TAG 7
#define ACK_TAG 8
#define ORDER_TAG 9
#define EXSUPPLY_TAG 10
#define EXSUPPLY_REQUEST_TAG 11
#define REPORT_TAG 12
#define COMMAND_TAG 13
#define END_TAG 14




int vCount(int *A,int c,int value){
    int i,count=0;
    for (i=0; i<c; i++){
        if (A[i] == value)
        count ++;
    }
    return count;
}



int search(int *A,int c,int value){
    int i;
    for (i=0; i<c; i++){
        if (A[i] == value)
        return 1;
    }
    return 0;
}





int main(int argc, char *argv[]){

    int server_count=0,iteration = 1,rank,world_size,proc,server=0,k=0,i=0,j=0,leader=0,l2=0,event_counter=0,asleep = 1,num2=0,q,received_amount=0,requested_amount=0,num,rec_num,orders[2],supply[3],order_num,c=0,count=0,client_search=1,supply_num;
    int childs_rec=0,total_sum=0,server_child_sum=0,received_child_sum=0,command=0,child_count,client_quan=0,client_rank,parent_rank=0,parent = 1,quantity=300,client=0,rec_client,childs[50],dummy=-1,order_clients[100],o=0,ex_supply,wanted_supply,child_sum;
    int servers[NUM_SERVERS],probe[3],reply[2],neighs[2],left,right,server_rank,rec_left,rec_right,childs_sum,ser=0;
    char buff[BUFFER_SIZE],*message;
    char *string,*le,*event;
    int replyL,replyR,clients[100],parents[100];


    FILE *fp = fopen(argv[2], "r");  
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    NUM_SERVERS = atoi(argv[1]);

    // MPI Initialization
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;
 



    if (rank == 0){
        while(read = getline(&line, &len, fp) != -1){
            string = strtok(line," ");

            if (strcmp(string,"SERVER") == 0){
                proc = atoi(strtok(NULL," "));
                neighs[0] = atoi(strtok(NULL," "));
                neighs[1] = atoi(strtok(NULL," "));

                // servers[ser] = proc;
                // ser++;

                MPI_Send(&neighs, 2, MPI_INT, proc, SERVER_TAG, MPI_COMM_WORLD);
                event_counter++;
                MPI_Recv(&message, 3, MPI_CHAR, proc , SERVER_TAG, MPI_COMM_WORLD, &status);        
                event_counter--;
                
            }else{ 
                neighs[1] = -1;
                for (i=1; i<world_size; ++i){
                    MPI_Send(&neighs, 2, MPI_INT, i, SERVER_TAG, MPI_COMM_WORLD);
                }
                break;
            }
        }
            
    }

    

/*----------------------SERVER-----------------------*/
    if(rank != 0){
        while(1){
            MPI_Recv(&neighs, 2, MPI_INT, 0, SERVER_TAG, MPI_COMM_WORLD, &status);
            if (neighs[1] == -1) break;
            server = 1;
            MPI_Send("ACK", 3, MPI_CHAR, 0, SERVER_TAG, MPI_COMM_WORLD);
            break;
        }
    }



/*--------------LEADER_ELECTION---------------*/

    if (rank == 0 ){

                
                for (i=0; i<ser; i++) {
                    printf("Send Election to server %d\n",servers[i]);
                    MPI_Send("Election", 8, MPI_CHAR, i, 0,MPI_COMM_WORLD);
                }
                
                MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, ACK_TAG, MPI_COMM_WORLD, &status);
                // leader = l2;
                printf("LEADER ELECTED <%d>\n",leader);
                 for (i=NUM_SERVERS+1; i<world_size; i++){ 
                    MPI_Send(&l2, 1, MPI_INT, i, CLIENT_TAG,MPI_COMM_WORLD);
                    MPI_Recv(&message, 3, MPI_CHAR, i, CLIENT_TAG, MPI_COMM_WORLD, &status);
                }  
    }


    if (rank != 0 && server){
        MPI_Recv(&message,8, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);    
        while(leader == 0){
            if (asleep == 1) {
                asleep = 0;
                probe[0] = rank; 
                probe[1] = 0;
                probe[2] = 1;
                MPI_Send(&probe, 3, MPI_INT, neighs[0], PROBE_TAG, MPI_COMM_WORLD);
                MPI_Send(&probe, 3, MPI_INT, neighs[1], PROBE_TAG, MPI_COMM_WORLD);
            } 

            MPI_Recv(&probe, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (probe[0] == -2){
                leader = probe[1];
                MPI_Send(&probe, 3, MPI_INT, neighs[1], PROBE_TAG, MPI_COMM_WORLD);
                break;
            }else
            if (status.MPI_SOURCE == neighs[0] && probe[2] != -1){
                if (probe[0] == rank) {
                    leader = rank;
                    probe[0] = -2;
                    probe[1] = rank;
                    MPI_Send(&probe, 3, MPI_INT, neighs[1], PROBE_TAG, MPI_COMM_WORLD);
                    MPI_Send(&leader, 1, MPI_INT, 0, ACK_TAG, MPI_COMM_WORLD);
                    break;
                }
                
                if (probe[0] > rank && probe[2] < pow(2,probe[1])){
                    probe[2] = probe[2] + 1;
                    MPI_Send(&probe, 3, MPI_INT, neighs[1], PROBE_TAG, MPI_COMM_WORLD);
                }
                if (probe[0] > rank && probe[2] >= pow(2,probe[1])){
                    probe[2] = -1;
                    MPI_Send(&probe, 3, MPI_INT, neighs[0], REPLY_TAG, MPI_COMM_WORLD);
                }


            }else if (status.MPI_SOURCE == neighs[1] && probe[2] != -1){
                if (probe[0] == rank) {
                    leader = rank;
                    probe[0] = -2;
                    probe[1] = rank;
                    MPI_Send(&probe, 3, MPI_INT, neighs[1], PROBE_TAG, MPI_COMM_WORLD);
                    MPI_Send(&leader, 1, MPI_INT, 0, ACK_TAG, MPI_COMM_WORLD);
                    break;
                }

                if (probe[0] > rank && probe[2] < pow(2,probe[1])){
                    probe[2] = probe[2] + 1;
                    MPI_Send(&probe, 3, MPI_INT, neighs[0], PROBE_TAG, MPI_COMM_WORLD);
                }
                if (probe[0] > rank && probe[2] >= pow(2,probe[1])){
                    probe[2] = -1;
                    MPI_Send(&probe, 3, MPI_INT, neighs[1], REPLY_TAG, MPI_COMM_WORLD);
                }


            }else if (status.MPI_SOURCE == neighs[0] && probe[2] == -1){
                if (probe[0] != rank) {
                    MPI_Send(&probe, 2, MPI_INT, neighs[1], REPLY_TAG, MPI_COMM_WORLD);
                }
                else {
                    rec_left = 1;
                    if (rec_right == 1){
                        probe[0] = rank;
                        probe[1] = probe[1] + 1;
                        probe[2] = 1;
                        MPI_Send(&probe, 3, MPI_INT, neighs[0], PROBE_TAG, MPI_COMM_WORLD);
                        MPI_Send(&probe, 3, MPI_INT, neighs[1], PROBE_TAG, MPI_COMM_WORLD);
                    }
                }
                

            }else if (status.MPI_SOURCE == neighs[1] && probe[2] == -1){
                if (probe[0] != rank) {
                    MPI_Send(&probe, 2, MPI_INT, neighs[0], REPLY_TAG, MPI_COMM_WORLD);
                }
                else {
                    rec_right = 1;
                    if (rec_left == 1){
                        probe[0] = rank;
                        probe[1] = probe[1] + 1;
                        probe[2] = 1;
                        MPI_Send(&probe, 3, MPI_INT, neighs[0], PROBE_TAG, MPI_COMM_WORLD);
                        MPI_Send(&probe, 3, MPI_INT, neighs[1], PROBE_TAG, MPI_COMM_WORLD);
                    }
                }
            }
            
        }
    }

    // if (rank == leader){
        // l2 = leader;
        // printf("LEADER ELECTED <%d>\n",leader);
        // MPI_Send(&l2, 1, MPI_INT, 0, ACK_TAG, MPI_COMM_WORLD);
    // }


    if (rank != 0 && server == 0){
        MPI_Recv(&l2, 1, MPI_INT, 0, CLIENT_TAG, MPI_COMM_WORLD, &status);
        client = 1;
        MPI_Send("ACK",3,MPI_CHAR,0,CLIENT_TAG,MPI_COMM_WORLD);
    }


  

 /*---------------------CONNECT--------------------------------*/
       if (rank == 0){

    
            while(read = getline(&line, &len, fp) != -1){
                string = strtok(line," ");

                if (strcmp(string,"CONNECT") == 0){
                    client_rank = atoi(strtok(NULL," "));
                    parent_rank = atoi(strtok(NULL," "));
                    clients[c] = client_rank;
                    parents[c] = parent_rank;
                    c++;
                }else break;
            }
       }

    MPI_Bcast(&c, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&clients, c, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&parents, c, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0){
        int i;
        for(i=0; i<c; i++){
            MPI_Send(&parents[i], 1, MPI_INT, clients[i], CLIENT_TAG, MPI_COMM_WORLD);
            event_counter++;
            MPI_Recv(&message,3,MPI_CHAR,clients[i],ACK_TAG ,MPI_COMM_WORLD, &status);
            event_counter--;
        }
    }
                
    if (rank !=0){
        if (client){
            MPI_Recv(&parent_rank, 1, MPI_INT, 0, CLIENT_TAG, MPI_COMM_WORLD, &status);
            MPI_Send(&rank, 1, MPI_INT, parent_rank, CLIENT_TAG, MPI_COMM_WORLD);
            MPI_Recv(&message, 3, MPI_CHAR, parent_rank, ACK_TAG, MPI_COMM_WORLD, &status);
            MPI_Send("ACK", 3, MPI_CHAR, 0, ACK_TAG, MPI_COMM_WORLD);
        }

        if (search(parents,c,rank)){
            count = vCount(parents,c,rank);
            for (i=0; i<count; ++i){
                MPI_Recv(&rec_client, 1, MPI_INT, MPI_ANY_SOURCE, CLIENT_TAG, MPI_COMM_WORLD, &status);
                childs[i] = rec_client;
                MPI_Send("ACK", 3, MPI_CHAR, rec_client, ACK_TAG, MPI_COMM_WORLD);
            }
        }
    }


    

    MPI_Barrier(MPI_COMM_WORLD);

  




    if (rank == 0){

        
        //for the last command
        sscanf(string,"%s",string);
        if (strcmp(string,"ORDER") == 0){
                event = "ORDER";
                command = 2;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
                client_rank = atoi(strtok(NULL," "));
                num = atoi(strtok(NULL," "));
                orders[0] = client_rank;
                orders[1] = num;
                MPI_Send(&orders,2,MPI_INT, client_rank, ORDER_TAG, MPI_COMM_WORLD);
                event_counter++;
                MPI_Recv(&message,3,MPI_CHAR, client_rank, ACK_TAG, MPI_COMM_WORLD, &status);
                event_counter--;
        }else if (strcmp(string,"PRINT") == 0){
                event = "PRINT";
                command = 1;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
            
            MPI_Send("PRINT", 5, MPI_CHAR, leader, PRINT_TAG, MPI_COMM_WORLD);
            MPI_Recv(&message, 3, MPI_CHAR, leader, ACK_TAG, MPI_COMM_WORLD, &status);
        }
        else if (strcmp(string,"SUPPLY") == 0){
                event = "SUPPL";
                command = 3;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
                supply[0] = atoi(strtok(NULL," "));
                supply[1] = atoi(strtok(NULL," "));
                supply[2] = supply[1];
                MPI_Send(&supply,3,MPI_INT,supply[0], SUPPLY_TAG, MPI_COMM_WORLD);
                event_counter++;
                MPI_Recv(&message,3,MPI_CHAR,supply[0], ACK_TAG, MPI_COMM_WORLD,&status);
                event_counter--;
            }
            else if (strcmp(string,"EXTERNAL_SUPPLY") == 0){
                event = "EXTER";
                command = 4;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
                ex_supply = atoi(strtok(NULL," "));
                MPI_Send(&ex_supply,1,MPI_INT,leader,EXSUPPLY_TAG,MPI_COMM_WORLD);
                MPI_Recv(&message,3,MPI_CHAR,leader,ACK_TAG,MPI_COMM_WORLD,&status); 

            }
            else if (strcmp(string,"REPORT") == 0){
                event = "REPOR";
                command = 5;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
                MPI_Send("REPORT",6, MPI_CHAR,leader,REPORT_TAG,MPI_COMM_WORLD);
                MPI_Recv(&message,3, MPI_CHAR,leader,END_TAG,MPI_COMM_WORLD,&status);
            }
        
        









        // for the rest of the testfile
        while(read = getline(&line, &len, fp) != -1){        
            string = strtok(line," ");
            sscanf(string,"%s",string);
            if (strcmp(string,"PRINT") == 0){
                event = "PRINT";
                command = 1;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
            
            MPI_Send("PRINT", 5, MPI_CHAR, leader, PRINT_TAG, MPI_COMM_WORLD);
            MPI_Recv(&message, 3, MPI_CHAR, leader, ACK_TAG, MPI_COMM_WORLD, &status);
            }
            else if (strcmp(string,"ORDER") == 0){
                event = "ORDER";
                command = 2;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
                client_rank = atoi(strtok(NULL," "));
                num = atoi(strtok(NULL," "));
                orders[0] = client_rank;
                orders[1] = num;
                MPI_Send(&orders,2,MPI_INT, client_rank, ORDER_TAG, MPI_COMM_WORLD);
                event_counter++;
                MPI_Recv(&message,3,MPI_CHAR, client_rank, ACK_TAG, MPI_COMM_WORLD, &status);
                event_counter--;
            }
            else if (strcmp(string,"SUPPLY") == 0){
                event = "SUPPL";
                command = 3;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
                supply[0] = atoi(strtok(NULL," "));
                supply[1] = atoi(strtok(NULL," "));
                supply[2] = supply[1];
                MPI_Send(&supply,3,MPI_INT,supply[0], SUPPLY_TAG, MPI_COMM_WORLD);
                event_counter++;
                MPI_Recv(&message,3,MPI_CHAR,supply[0], ACK_TAG, MPI_COMM_WORLD,&status);
                event_counter--;
            }
            else if (strcmp(string,"EXTERNAL_SUPPLY") == 0){
                event = "EXTER";
                command = 4;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
                ex_supply = atoi(strtok(NULL," "));
                MPI_Send(&ex_supply,1,MPI_INT,leader,EXSUPPLY_TAG,MPI_COMM_WORLD);
                MPI_Recv(&message,3,MPI_CHAR,leader,ACK_TAG,MPI_COMM_WORLD,&status); 

            }
            else if (strcmp(string,"REPORT") == 0){
                event = "REPOR";
                command = 5;
                for (i=1; i< world_size; ++i)
                    MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);
                MPI_Send("REPORT",6, MPI_CHAR,leader,REPORT_TAG,MPI_COMM_WORLD);
                MPI_Recv(&message,3, MPI_CHAR,leader,END_TAG,MPI_COMM_WORLD,&status);
            }

        }
        command = -1;
        for (i=1; i< world_size; ++i)
            MPI_Send(&command,1, MPI_INT,i,COMMAND_TAG,MPI_COMM_WORLD);    

    }   



    while (command != -1){
        MPI_Recv(&command, 1 ,MPI_INT, 0 ,COMMAND_TAG, MPI_COMM_WORLD,&status);
        

        if (command == 1){
// ----------------------------------------PRINT-------------------------------------------
            if (server){
                if(rank == leader) {
                    MPI_Recv(&message, 5, MPI_CHAR, 0, PRINT_TAG, MPI_COMM_WORLD, &status);
                    printf("SERVER <%d>  HAS QUANTITY <%d>\n",rank,quantity);
                    MPI_Send(&message,5,MPI_CHAR, neighs[0], PRINT_TAG, MPI_COMM_WORLD);
                }
                else 
                {
                    MPI_Recv(&message, 5, MPI_CHAR, neighs[1], PRINT_TAG, MPI_COMM_WORLD, &status);
                    printf("SERVER <%d>  HAS QUANTITY <%d>\n",rank,quantity);
                    MPI_Send(&message,5,MPI_CHAR, neighs[0], PRINT_TAG, MPI_COMM_WORLD);
                }


                if (rank == leader){
                    MPI_Recv(&message, 5, MPI_CHAR, neighs[1], PRINT_TAG, MPI_COMM_WORLD, &status);
                    MPI_Send("ACK",3,MPI_CHAR, 0, ACK_TAG, MPI_COMM_WORLD);
                }
            }




        }else if (command == 2){
            
//----------------------------------ORDER-------------------------------------

            while(1){
                MPI_Recv(&orders,2,MPI_INT, MPI_ANY_SOURCE, ORDER_TAG, MPI_COMM_WORLD, &status);
                if (orders[0] == -1) {
                    break;
                }
                if (parent_rank != 0){
                    MPI_Send(&orders,2,MPI_INT,parent_rank, ORDER_TAG, MPI_COMM_WORLD);    
                }   
                else  {
                    quantity = quantity - orders[1];
                    MPI_Send("ACK",3,MPI_CHAR,orders[0], ACK_TAG, MPI_COMM_WORLD);    
                }
        
                if (orders[0] == rank){
                    MPI_Recv(&message,3,MPI_CHAR, MPI_ANY_SOURCE, ACK_TAG, MPI_COMM_WORLD, &status);
                    client_quan = client_quan + orders[1];
                    printf("CLIENT <%d> SOLD <%d>\n",rank,orders[1]);
                    MPI_Send("ACK",3,MPI_CHAR,0, ACK_TAG, MPI_COMM_WORLD);
                    orders[0] = -1;
                    for (i=1; i<world_size; i++){
                        if (i!=rank) MPI_Send(&orders,2,MPI_INT,i, ORDER_TAG, MPI_COMM_WORLD);
                    }
                    break;
                }
            }





        }else if (command == 3){
//-------------------------------------------SUPPLY-----------------------------------
            if (server){
                while (1){
                    MPI_Recv(&supply,3,MPI_INT,MPI_ANY_SOURCE, SUPPLY_TAG, MPI_COMM_WORLD,&status);
                    
                    if (supply[0] == -1) break;

                    if (status.MPI_SOURCE == 0){
                        MPI_Send(&supply,3,MPI_INT,neighs[0],SUPPLY_TAG, MPI_COMM_WORLD);
                        requested_amount = supply[1];
                        received_amount = 0;
                        continue;
                    }
                    if (supply[0] != rank){
                        if (quantity > 150 + supply[1]){
                            quantity = quantity - supply[1];
                            supply[2] = supply[2] - supply[1];
                            MPI_Send(&supply,3,MPI_INT,supply[0],SUPPLY_TAG, MPI_COMM_WORLD);
                        }else if (quantity > 150 && quantity <= (150 + supply[1])){
                            q = quantity-150;
                            supply[1] = q;
                            quantity = 150;
                            supply[2] = supply[2] - supply[1];
                            MPI_Send(&supply,3,MPI_INT,supply[0],SUPPLY_TAG, MPI_COMM_WORLD);
                            
                            if (neighs[0] != supply[0] && supply[2]>0) {
                            supply[1] = supply[2];
                            MPI_Send(&supply,3,MPI_INT,neighs[0],SUPPLY_TAG, MPI_COMM_WORLD);
                            }
                        }else if (quantity <= 150){
                            MPI_Send(&supply,3,MPI_INT,neighs[0],SUPPLY_TAG, MPI_COMM_WORLD);
                        }


                    } else {
                        received_amount = received_amount + supply[1];
                        quantity = quantity + supply[1];
                        if (received_amount >= requested_amount || status.MPI_SOURCE == neighs[1]){
                            printf("SERVER <%d> RECEIVED <%d>\n",rank,received_amount,quantity);
                            MPI_Send("ACK",3,MPI_CHAR,0,ACK_TAG, MPI_COMM_WORLD);
                            supply[0] = -1;
                            for (i=1; i<world_size; i++){
                                if (i!=rank) MPI_Send(&supply,3,MPI_INT,i, SUPPLY_TAG, MPI_COMM_WORLD);
                            }
                            break;
                        }
                    }
                }
                
            }


        }else if (command == 4){
// ----------------------------------------EXTERNAL_SUPPLY------------------------------------------
            if (server){
                if (rank == leader){
                    MPI_Recv(&ex_supply,1,MPI_INT,0,EXSUPPLY_TAG,MPI_COMM_WORLD, &status);
                    if (quantity < 150){
                        ex_supply = ex_supply-150+quantity;
                        quantity = 150;
                    }
                    MPI_Send(&wanted_supply,1,MPI_INT,neighs[0],EXSUPPLY_TAG,MPI_COMM_WORLD);
                    while (1){
                        MPI_Recv(&wanted_supply,1,MPI_INT,MPI_ANY_SOURCE,EXSUPPLY_REQUEST_TAG,MPI_COMM_WORLD, &status);
                        // printf("Received wanted supply from %d\n",status.MPI_SOURCE);

                        if (wanted_supply == -1){
                            MPI_Send("OKAY",4,MPI_CHAR,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);
                            MPI_Recv(&message,8,MPI_CHAR,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
                            if (status.MPI_SOURCE != neighs[1]) continue;        
                        }else{

                        if (ex_supply >= wanted_supply){
                            ex_supply = ex_supply - wanted_supply;
                            MPI_Send(&wanted_supply,1,MPI_INT,status.MPI_SOURCE,EXSUPPLY_TAG,MPI_COMM_WORLD);        
                        }else{
                            // If supply is not enough
                            wanted_supply = ex_supply;
                            ex_supply = 0;
                            MPI_Send(&wanted_supply,1,MPI_INT,status.MPI_SOURCE,EXSUPPLY_TAG,MPI_COMM_WORLD);        
                        }
                        MPI_Recv(&message,8,MPI_CHAR,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
                        }
                        
                        
                        if (status.MPI_SOURCE == neighs[1]){    
                            // if ex_supply surpluses
                            if (ex_supply > 0){
                                quantity = quantity + ex_supply;
                                ex_supply = 0;
                            }
                            break;
                        }
                    }
                    printf("LEADER SUPPLY <%d> OK\n",quantity);
                    MPI_Send("ACK",3,MPI_CHAR,0,ACK_TAG,MPI_COMM_WORLD);        
                }else{
                    MPI_Recv(&wanted_supply,1,MPI_INT,neighs[1],EXSUPPLY_TAG,MPI_COMM_WORLD, &status);
                    if (quantity < 150) {
                        wanted_supply = 150-quantity;
                        MPI_Send(&wanted_supply,1,MPI_INT,leader,EXSUPPLY_REQUEST_TAG,MPI_COMM_WORLD);
                        MPI_Recv(&wanted_supply,1,MPI_INT,leader,EXSUPPLY_TAG,MPI_COMM_WORLD, &status);
                        printf("Rank %d received supply %d from leader\n",rank,wanted_supply);
                        quantity = quantity + wanted_supply;
                    }else{
                        wanted_supply = -1;
                        MPI_Send(&wanted_supply,1,MPI_INT,leader,EXSUPPLY_REQUEST_TAG,MPI_COMM_WORLD);
                        MPI_Recv(&message,4,MPI_CHAR,leader,ACK_TAG,MPI_COMM_WORLD, &status);
                    }
                    MPI_Send("Received",8,MPI_CHAR,leader,ACK_TAG,MPI_COMM_WORLD);
                    MPI_Send(&wanted_supply,1,MPI_INT,neighs[0],EXSUPPLY_TAG,MPI_COMM_WORLD);
                }
            }
        }else if (command == 5){
// ------------------------------------------------REPORT----------------------------------------
            if (server){
                if (rank == leader){
                    MPI_Recv(&message,6,MPI_CHAR,MPI_ANY_SOURCE,REPORT_TAG,MPI_COMM_WORLD, &status);

                    
                    

                    MPI_Send(&message,6,MPI_CHAR,neighs[0],REPORT_TAG,MPI_COMM_WORLD);
                    
                    for (i=0; i<NUM_SERVERS-1; ++i){
                        MPI_Recv(&server_child_sum,6,MPI_INT,MPI_ANY_SOURCE,REPORT_TAG,MPI_COMM_WORLD, &status);
                        total_sum = total_sum + server_child_sum;
                    }

                    for (i=0; i<count; i++){
                            MPI_Send(&server_child_sum,1,MPI_INT,childs[i],REPORT_TAG,MPI_COMM_WORLD);
                            MPI_Recv(&server_child_sum,1,MPI_INT,childs[i],REPORT_TAG,MPI_COMM_WORLD, &status);
                    }
                    printf("REPORT <%d> <%d>\n",rank,total_sum);
                    MPI_Send("END",3,MPI_CHAR,0,END_TAG,MPI_COMM_WORLD);
                
                }else{
                    MPI_Recv(&message,6,MPI_CHAR,neighs[1],REPORT_TAG,MPI_COMM_WORLD, &status);

                    for (i=0; i<count; i++){
                            MPI_Send(&server_child_sum,1,MPI_INT,childs[i],REPORT_TAG,MPI_COMM_WORLD);
                            MPI_Recv(&server_child_sum,1,MPI_INT,childs[i],REPORT_TAG,MPI_COMM_WORLD, &status);
                        }
                    printf("REPORT <%d> <%d>\n",rank,server_child_sum);

                    if (neighs[0] != leader) MPI_Send(&message,6,MPI_CHAR,neighs[0],REPORT_TAG,MPI_COMM_WORLD);
                    MPI_Send(&server_child_sum,1,MPI_INT,leader,REPORT_TAG,MPI_COMM_WORLD);
                }
            }else{
                    //Client
                    while(1){
                    MPI_Recv(&server_child_sum,1,MPI_INT,MPI_ANY_SOURCE,REPORT_TAG,MPI_COMM_WORLD,&status);

                    if (status.MPI_SOURCE == parent_rank){
                        server_child_sum = server_child_sum + client_quan;
                    }
                

                    if (count == 0){
                        if (parent_rank != 0){
                            MPI_Send(&server_child_sum,1,MPI_INT,parent_rank,REPORT_TAG,MPI_COMM_WORLD);
                            break;
                        }
                    }

                    for(i=0; i<count; i++){
                        //Send to client childs
                        MPI_Send(&server_child_sum,1,MPI_INT,childs[i],REPORT_TAG,MPI_COMM_WORLD);
                        MPI_Recv(&server_child_sum,1,MPI_INT,childs[i],REPORT_TAG,MPI_COMM_WORLD,&status);
                    }

                    if (status.MPI_SOURCE != parent_rank){
                        MPI_Send(&server_child_sum,1,MPI_INT,parent_rank,REPORT_TAG,MPI_COMM_WORLD); 
                        break;
                    }

                    }                    

                }

        }
    }
    

    MPI_Finalize();
    return 0;
}




