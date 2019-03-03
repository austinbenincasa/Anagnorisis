#include <string>
#include <map>
#include <chrono>
#include <thread>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

enum AnagnorisisResponse {
    OK = 0,
    IDERROR = 1,
    IDMISMATCH = 2,
    NAMEERROR = 3,
    NORECORD = 4,
    NORECORDID = 5,
    RECORDEXISTS = 6,
    NOSERVER = 7,
    INVALIDRESP = 8,
    INVALIDREQ = 9,   
    SSLERROR = 10,
    CLIENTERROR = 11,
    APIERROR = 12
};

class Anagnorisis {
    public:
        Anagnorisis(std::string, int, int, int);
        ~Anagnorisis();
        AnagnorisisResponse retrieveRecord(std::string, std::string&);
        AnagnorisisResponse removeRecord(std::string);
        AnagnorisisResponse createRecord(std::string, std::string);
        AnagnorisisResponse updateRecord(std::string, std::string);
        std::string getAnagnorisisResponse(AnagnorisisResponse);
    private:
        void ping();
        int threads;
        int max_con;
        int timeout;
        bool running = true;
        std::string url;
        std::thread pingThread;
        std::map<std::string, std::string> record_ids;
};