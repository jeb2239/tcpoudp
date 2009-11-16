/*********************************************************
 * io management for congetion control 
 * note: thread per socket 
 * ******************************************************/
extern boost::mutex sndqmutex;

clasn ioCongMng {
  public:
    ioCongMng()
    :i_thread(boost::bind(&iocongmng::run, this))
    {
      std::cout<< "*** iocongmng is running now ***\n";
    }
    ~ioCongMng(){
      std::cout << "*** iocongmng is recycled *** " << std::endl;
      m_thread.join(); // RAII designed pattern, recycling self-managment
    }

    void pushsndq(touheaderack *touack);
    //pushrecvq(touheaderack *touack);

  private:
    std::queue<struct touheaderack>		sndq;
    std::queue<struct touheaderack>		iterator;
    boost::thread				i_thread;
    int						sockfd;
    touheaderack			 	touhdack;	
    struct sockaddr_in				sockaddr;
    
    /* error return: 0
     * success return: 1 */
    int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, char *ip, char *port); /* get the sockaddr_in infomation */
    void popsndq(tou);
    void run();


};
