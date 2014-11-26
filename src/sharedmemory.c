#include "sharedmemory.h"
#include "sharedmemory_global.h"

void Sharedmemory_init(void) 
{

  int bindport = getIntConf(BINDPORT);                //MJP BUG commandline option for address and port dont work this way going to have 
  int server_max_clients = getIntConf(MAX_CLIENTS);   //to bring them across as prameters to Sharedmemory_init(void)
  int shmptr_size =  sizeof( shm_t  ) + (sizeof( shmclient_t ) * server_max_clients);

  sprintf( shm_file_name, "umurmurd:%i", bindport );
  Log_info("SHM_API: shm_fd=\"%s\"", shm_file_name  );

		shm_fd = shm_open( shm_file_name, O_CREAT | O_RDWR, 0666 );
				if(shm_fd == -1)
				{
    				Log_fatal( "SHM_API: Open failed:%s\n", strerror(errno));
    				exit(1);
				}  

				if( ftruncate( shm_fd, shmptr_size ) == -1 )
				{
    				Log_fatal( "SHM_API: ftruncate : %s\n", strerror(errno));  
    				exit(1);
				}

  			shmptr = mmap(0, shmptr_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  			if (shmptr == (void *) -1) 
  			{
     				Log_fatal( "SHM_API: mmap failed : %s\n", strerror(errno));
     				exit(1);
  			} 

  memset( shmptr, 0, shmptr_size );
                                       
  shmptr->umurmurd_pid = getpid();
  shmptr->server_max_clients = server_max_clients;  
}

void Sharedmemory_update(void) 
{

  uint64_t now;
  unsigned int cc = 0;
  client_t *client_itr = NULL;

    memset( &shmptr->client[0], 0, sizeof( shmclient_t ) * shmptr->server_max_clients );
    shmptr->clientcount = Client_count();
    
      if( shmptr->clientcount )
      {
        Timer_init( &now );
          while( Client_iterate(&client_itr) != NULL )
          {                                                                                   
            if( client_itr->authenticated )
            {        
              channel_t *channel = client_itr->channel;
        
                strncpy( shmptr->client[cc].username, client_itr->username, 120 );
                strncpy( shmptr->client[cc].ipaddress, Util_clientAddressToString( client_itr ), 45 );
                strncpy( shmptr->client[cc].channel, channel->name, 120 );
                
                strncpy( shmptr->client[cc].os, client_itr->os, 120 );
                strncpy( shmptr->client[cc].release, client_itr->release, 120 );
                strncpy( shmptr->client[cc].os_version, client_itr->os_version, 120 );
                
                shmptr->client[cc].tcp_port = Util_clientAddressToPortTCP( client_itr );
                shmptr->client[cc].udp_port = Util_clientAddressToPortUDP( client_itr );                
        
                shmptr->client[cc].online_secs = ( now - client_itr->connectTime ) / 1000000LL;
                shmptr->client[cc].idle_secs = ( now - client_itr->idleTime ) / 1000000LL;
                        
                shmptr->client[cc].bUDP  = client_itr->bUDP;
                shmptr->client[cc].deaf  = client_itr->deaf;
                shmptr->client[cc].mute  = client_itr->mute;
                shmptr->client[cc].bOpus  = client_itr->bOpus;
                shmptr->client[cc].self_deaf  = client_itr->self_deaf;
                shmptr->client[cc].self_mute  = client_itr->self_mute;
                shmptr->client[cc].recording  = client_itr->recording;
                shmptr->client[cc].authenticated  = client_itr->authenticated;
                
                shmptr->client[cc].availableBandwidth  = client_itr->availableBandwidth;
                
                shmptr->client[cc].UDPPingAvg = client_itr->UDPPingAvg;
                shmptr->client[cc].UDPPingVar = client_itr->UDPPingVar;
                shmptr->client[cc].TCPPingAvg = client_itr->TCPPingAvg;
                shmptr->client[cc].TCPPingVar = client_itr->TCPPingVar;
                
                shmptr->client[cc].isAdmin = client_itr->isAdmin;
                shmptr->client[cc].isSuppressed = client_itr->isSuppressed;
                
                shmptr->client[cc].UDPPackets = client_itr->UDPPackets;
                shmptr->client[cc].TCPPackets = client_itr->TCPPackets;
                
            }  
          cc++;     
        }
      } 
}
void Sharedmemory_alivetick(void)
{
 shmptr->alive++;
}

void Sharedmemory_deinit(void) 
{
  close( shm_fd );
  shm_unlink( shm_file_name );
  shmptr->umurmurd_pid = 0;
}
