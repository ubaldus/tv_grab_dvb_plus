
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "libsi/util.h"
#include "libsi/descriptor.h"
#include "libsi/section.h"
#include "dvb_epg.h"
#include "dvb_info.h"
#include "dvb_text.h"
#include "dvb_parse.h"
#include "lookup.h"
#include "chanid.h"
#include "stats.h"
#include "log.h"

char *compactspace(char *s);

void parseSDT(u_char* data, int len, sChannel** sdtChannels)
{
    SI::SDT sdt(data, false);
    if (!sdt.CheckCRCAndParse())
        return;

    SI::SDT::Service SiSdtService;
    for (SI::Loop::Iterator it; sdt.serviceLoop.getNext(SiSdtService, it);) {

        sChannel *C, Key;
        Key.ChannelId = Key.Sid = SiSdtService.getServiceId();
        Key.Nid = sdt.getOriginalNetworkId();
        Key.Tid = sdt.getTransportStreamId();

        if (*sdtChannels != NULL)
        {
            sChannel *Cn;
            for(C=*sdtChannels; C != NULL; C = C->next)
            {
                if (C->Sid == Key.Sid)
                    return;
                if (Key.Sid > C->Sid)
                {
                    break;
                }
            }
            Cn = new sChannel();
            *Cn = Key;
            if (C != NULL)
            {
                Cn->next = C->next;
                C->next = Cn;
            }
            else
            {
                Cn->next = C;
                *sdtChannels = C;
            }
            C = Cn;
        }
        else
        {
            C = *sdtChannels = new sChannel();
            *C = Key;
        }

#if 0
        if (firstSDTChannel == NULL) {
            firstSDTChannel = C;
        } else if (firstSDTChannel == C) {
            if (nChannelUpdates == 0) {
                EndSDT = true;
            } else
                nChannelUpdates = 0;
        }
#endif

        log_message(TRACE, "new channel %d %d %d", C->Nid, C->Tid, C->Sid);

        SI::Descriptor * d;
        for (SI::Loop::Iterator it2; (d = SiSdtService.serviceDescriptors.getNext(it2));) {
            log_message(TRACE, " tag 0x%x", d->getDescriptorTag());
            switch (d->getDescriptorTag()) {
                case SI::ServiceDescriptorTag:
                    {
                        SI::ServiceDescriptor * sd = (SI::ServiceDescriptor *) d;
                        log_message(TRACE, "  type 0x%x", sd->getServiceType());
                        switch (sd->getServiceType()) {
                            case 0x01:	// digital television service
                            case 0x02:	// digital radio sound service
                            case 0x04:	// NVOD reference service
                            case 0x05:	// NVOD time-shifted service
                                {
                                    char NameBuf[1024];
                                    char ShortNameBuf[1024];
                                    char ProviderNameBuf[1024];
                                    //log_message(TRACE, "B %02x %x-%x %x-%x %x-%x",
                                    //        sd->getServiceType(), Key.Nid, lChannels[10].Nid, Key.Tid, lChannels[10].Tid, Key.Sid, lChannels[10].Sid);
                                    sd->serviceName.getText(NameBuf, ShortNameBuf, sizeof(NameBuf), sizeof(ShortNameBuf));
                                    char *pn = compactspace(NameBuf);
                                    sd->providerName.getText(ProviderNameBuf, sizeof(ProviderNameBuf));
                                    char *provname = compactspace(ProviderNameBuf);
                                    log_message(TRACE, "   name %s prov %s", pn, provname);
                                    if (C) {
                                        if (C->name == NULL) {
                                            asprintf(&C->name, "%s", pn);
                                            asprintf(&C->providername, "%s", provname);
                                            log_message(DEBUG, "found channel %d %d %d '%s' '%s'", C->Nid, C->Tid, C->Sid, C->name, C->providername);
                                        }
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case SI::MultilingualServiceNameDescriptorTag:
                    {
                        if (C == NULL)
                            break;
                        SI::MultilingualServiceNameDescriptor * md = (SI::MultilingualServiceNameDescriptor *) d;
                        SI::MultilingualServiceNameDescriptor::Name n;
                        for (SI::Loop::Iterator it2; (md->nameLoop.getNext(n, it2));) {
                            // languageCode char[4]
                            // name String
                            if (strncmp(n.languageCode, "aka", 3) == 0) {
                                if (C->shortname == NULL) {
                                    char b[100];
                                    n.name.getText(b, sizeof(b));
                                    C->shortname = strdup(b);
                                }
                            } else {
                                if (!C->IsNameUpdated) {
                                    if (C->name) {
                                        free(C->name);
                                        C->name = NULL;
                                    }
                                    char b[100];
                                    n.name.getText(b, sizeof(b));
                                    C->name = strdup(b);
                                    C->IsNameUpdated = true;
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

