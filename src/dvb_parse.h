#ifndef __DVBPARSE_H__
#define __DVBPARSE_H__

class sChannel
{
    public:
        sChannel * next;
        unsigned short int ChannelId;
        unsigned short int Nid;
        unsigned short int Tid;
        unsigned short int Sid;
        char * name;
        char * shortname;
        char * providername;
        unsigned int pData;
        unsigned int lenData;
        bool IsFound;
        bool IsEpg;
        bool IsNameUpdated;

        sChannel() {
            memset(this, 0, sizeof(sChannel));
        }
        ~sChannel() {
            if (name)
                free(name);
            if (shortname)
                free(shortname);
            if (providername)
                free(providername);
        }
};

void parseSDT(u_char* data, int len, sChannel** channelHead);


#endif

