DRAM:4021C6EC huffman_word_lookup_lo:dd 0             ; DATA XREF: huffman_init_word_decode_tree+Fr
DRAM:4021C6F0                 dd 0
DRAM:4021C6F4                 dd 0
DRAM:4021C6F8                 dd 0
DRAM:4021C6FC huffman_word_lookup_hi:dd 0             ; DATA XREF: huffman_init_word_decode_tree+19r
DRAM:4021C6FC                                         ; huffman_func_decode__+90r
DRAM:4021C700                 dd 0
DRAM:4021C704                 dd 0
DRAM:4021C708                 dd 0
DRAM:4021C70C huffman_lookup_2:dd 0                   ; DATA XREF: huffman_init_word_decode_tree+2Ar
DRAM:4021C70C                                         ; huffman_init_word_decode_tree+36r
DRAM:4021C710                 dd 0
DRAM:4021C714                 dd 0
DRAM:4021C718                 dd 0
DRAM:4021C71C huffman_hi_bit_branch:dd 0              ; DATA XREF: huffman_init_decode_tree+1Dr
DRAM:4021C71C                                         ; huffman_func_decode__+27r
DRAM:4021C720                 dd 0
DRAM:4021C724                 dd 0
DRAM:4021C728                 dd 0
DRAM:4021C72C huffman_lo_bit_branch:dd 0              ; DATA XREF: huffman_init_decode_tree+13r
DRAM:4021C72C                                         ; huffman_func_decode__+31r
DRAM:4021C730                 dd 0
DRAM:4021C734                 dd 0
DRAM:4021C738                 dd 0
DRAM:4021C73C huffman_valid_start_code:dd 0           ; DATA XREF: huffman_init_decode_tree+2Er
DRAM:4021C73C                                         ; huffman_init_decode_tree+39r
DRAM:4021C73C                                         ; huffman_func_decode__+1Cr
DRAM:4021C740                 dd 0
DRAM:4021C744                 dd 0
DRAM:4021C748                 dd 0




DRAM:401AAB00 huffman_code_check:db 1                 ; DATA XREF: init_or_decode_huffman+8r
DRAM:401AAB00                                         ; init_or_decode_huffman+11r
DRAM:401AAB01                 db 0
DRAM:401AAB02                 dw 3907h

DRAM:401AAB04 huffman_word_dictionary:db "(Including ",0
DRAM:401AAB04                                         ; DATA XREF: init_or_decode_huffman+3Cr
DRAM:401AAB10 aNewSeries:     db "(New Series)",0
DRAM:401AAB1D aPart:          db "(Part ",0
DRAM:401AAB24 aRepeat:        db "(Repeat)",0
DRAM:401AAB2D aStereo:        db "(Stereo)",0
DRAM:401AAB36 aStereoTeletext:db "(Stereo) (Teletext)",0
DRAM:401AAB4A aTeletext:      db "(Teletext)",0
DRAM:401AAB55 aWidescreen:    db "(Widescreen)",0
DRAM:401AAB62 aAction:        db "Action",0
DRAM:401AAB69 aAdventures:    db "Adventures",0
DRAM:401AAB74 aAmerica:       db "America",0
DRAM:401AAB7C aAnimated:      db "Animated",0
DRAM:401AAB85 aAustralia:     db "Australia",0
DRAM:401AAB8F aAway:          db "Away",0
DRAM:401AAB94 aBbc:           db "BBC",0
DRAM:401AAB98 aBaby:          db "Baby",0
DRAM:401AAB9D aBest:          db "Best",0
DRAM:401AABA2 aBig:           db "Big",0
DRAM:401AABA6 aBill:          db "Bill",0
DRAM:401AABAB aBlack:         db "Black",0
DRAM:401AABB1 aBlue:          db "Blue",0
DRAM:401AABB6 aBreakfast:     db "Breakfast",0
DRAM:401AABC0 aBritain:       db "Britain",0
DRAM:401AABC8 aBritish:       db "British",0
DRAM:401AABD0 aBusiness:      db "Business",0
DRAM:401AABD9 aCall:          db "Call",0
DRAM:401AABDE aCartoon:       db "Cartoon",0
DRAM:401AABE6 aChannel:       db "Channel",0
DRAM:401AABEE aChildren:      db "Children",0
DRAM:401AABF7 aClock:         db "Clock",0
DRAM:401AABFD aComedy:        db "Comedy",0
DRAM:401AAC04 aCook:          db "Cook",0
DRAM:401AAC09 aCountry:       db "Country",0
DRAM:401AAC11 aDirectedBy:    db "Directed by ",0
DRAM:401AAC1E aDrama:         db "Drama",0
DRAM:401AAC24 aEast:          db "East",0
DRAM:401AAC29 aEducation:     db "Education",0
DRAM:401AAC33 aEnglish:       db "English",0
DRAM:401AAC3B aEurope:        db "Europe",0
DRAM:401AAC42 aExtra:         db "Extra",0
DRAM:401AAC48 aFinal:         db "Final",0
DRAM:401AAC4E aFinancial:     db "Financial",0
DRAM:401AAC58 aFor:           db "For",0
DRAM:401AAC5C aFrench:        db "French",0
DRAM:401AAC63 aFrom:          db "From",0
DRAM:401AAC68 aGeorge:        db "George",0
DRAM:401AAC6F aGet:           db "Get",0
DRAM:401AAC73 aGirls:         db "Girls",0
DRAM:401AAC79 aGolden:        db "Golden",0
DRAM:401AAC80 aGolf:          db "Golf",0
DRAM:401AAC85 aGood:          db "Good",0
DRAM:401AAC8A aGreat:         db "Great",0
DRAM:401AAC90 aHampshire:     db "Hampshire",0
DRAM:401AAC9A aHeadlines:     db "Headlines",0
DRAM:401AACA4 aHear:          db "Hear",0
DRAM:401AACA9 aHill:          db "Hill",0
DRAM:401AACAE aHollywood:     db "Hollywood",0
DRAM:401AACB8 aHome:          db "Home",0
DRAM:401AACBD aHour:          db "Hour",0
DRAM:401AACC2 aHouse:         db "House",0
DRAM:401AACC8 aHow:           db "How",0
DRAM:401AACCC aItn:           db "ITN",0
DRAM:401AACD0 aImportant:     db "Important",0
DRAM:401AACDA aIncluding_0:   db "Including",0
DRAM:401AACE4 aInternational: db "International",0
DRAM:401AACF2 aJohn:          db "John",0
DRAM:401AACF7 aLast:          db "Last",0
DRAM:401AACFC aLate:          db "Late",0
DRAM:401AAD01 aLearn:         db "Learn",0
DRAM:401AAD07 aLittle:        db "Little",0
DRAM:401AAD0E aLive:          db "Live",0
DRAM:401AAD13 aLondon:        db "London",0
DRAM:401AAD1A aLook:          db "Look",0
DRAM:401AAD1F aLunch:         db "Lunch",0
DRAM:401AAD25 aMan:           db "Man",0
DRAM:401AAD29 aMark:          db "Mark",0
DRAM:401AAD2E aMeridian:      db "Meridian",0
DRAM:401AAD37 aMichael:       db "Michael",0
DRAM:401AAD3F aMinutes:       db "Minutes",0
DRAM:401AAD47 aMore:          db "More",0
DRAM:401AAD4C aMorning:       db "Morning",0
DRAM:401AAD54 aMurder:        db "Murder",0
DRAM:401AAD5B aNation:        db "Nation",0
DRAM:401AAD62 aNeighbours:    db "Neighbours",0
DRAM:401AAD6D aNew:           db "New",0
DRAM:401AAD71 aNewsWeather:   db "News & Weather",0
DRAM:401AAD80 aNewsAndWeather:db "News And Weather",0
DRAM:401AAD91 aPaul:          db "Paul",0
DRAM:401AAD96 aPlus:          db "Plus",0
DRAM:401AAD9B aPrayer:        db "Prayer",0
DRAM:401AADA2 aPresent:       db "Present",0
DRAM:401AADAA aPresentedBy:   db "Presented by",0
DRAM:401AADB7 aQuiz:          db "Quiz",0
DRAM:401AADBC aRegional:      db "Regional",0
DRAM:401AADC5 aRepresent:     db "Represent",0
DRAM:401AADCF aResource:      db "Resource",0
DRAM:401AADD8 aReview:        db "Review",0
DRAM:401AADDF aRichard:       db "Richard",0
DRAM:401AADE7 aSchool:        db "School",0
DRAM:401AADEE aSeries:        db "Series",0
DRAM:401AADF5 aService:       db "Service",0
DRAM:401AADFD aShow:          db "Show",0
DRAM:401AAE02 aSmith:         db "Smith",0
DRAM:401AAE08 aSouth:         db "South",0
DRAM:401AAE0E aSport:         db "Sport",0
DRAM:401AAE14 aStar:          db "Star",0
DRAM:401AAE19 aStreet:        db "Street",0
DRAM:401AAE20 aTv:            db "TV",0
DRAM:401AAE23 aTeaching:      db "Teaching",0
DRAM:401AAE2C aThe:           db "The",0
DRAM:401AAE30 aToday:         db "Today",0
DRAM:401AAE36 aTonight:       db "Tonight",0
DRAM:401AAE3E aWeather:       db "Weather",0
DRAM:401AAE46 aWestern:       db "Western",0
DRAM:401AAE4E aWestminster:   db "Westminster",0
DRAM:401AAE5A aWilliam:       db "William",0
DRAM:401AAE62 aWith:          db "With",0
DRAM:401AAE67 aWorld:         db "World",0
DRAM:401AAE6D aAbout:         db "about",0
DRAM:401AAE73 aActionPacked:  db "action-packed",0
DRAM:401AAE81 aAdventure:     db "adventure",0
DRAM:401AAE8B aAfternoon:     db "afternoon",0
DRAM:401AAE95 aAlert:         db "alert",0
DRAM:401AAE9B aAllStarCast:   db "all-star cast",0
DRAM:401AAEA9 aAnd:           db "and",0
DRAM:401AAEAD aAnywhere:      db "anywhere",0
DRAM:401AAEB6 aAudience:      db "audience",0
DRAM:401AAEBF aBased:         db "based",0
DRAM:401AAEC5 aBook:          db "book",0
DRAM:401AAECA aBusiness_0:    db "business",0
DRAM:401AAED3 aBut:           db "but",0
DRAM:401AAED7 aCelebrity:     db "celebrity",0
DRAM:401AAEE1 aChance:        db "chance",0
DRAM:401AAEE8 aChat:          db "chat",0
DRAM:401AAEED aChild:         db "child",0
DRAM:401AAEF3 aClassic:       db "classic",0
DRAM:401AAEFB aConsumer:      db "consumer",0
DRAM:401AAF04 aContestants:   db "contestants",0
DRAM:401AAF10 aContinues:     db "continues",0
DRAM:401AAF1A aControversial: db "controversial",0
DRAM:401AAF28 aDealer:        db "dealer",0
DRAM:401AAF2F aDeliver:       db "deliver",0
DRAM:401AAF37 aDiscuss:       db "discuss",0
DRAM:401AAF3F aDocument:      db "document",0
DRAM:401AAF48 aDrama_0:       db "drama",0
DRAM:401AAF4E aEdition:       db "edition",0
DRAM:401AAF56 aEducation_0:   db "education",0
DRAM:401AAF60 aEvents:        db "events",0
DRAM:401AAF67 aEvery:         db "every",0
DRAM:401AAF6D aExcellent:     db "excellent",0
DRAM:401AAF77 aEyed:          db "eyed",0
DRAM:401AAF7C aFamily:        db "family",0
DRAM:401AAF83 aFamous:        db "famous",0
DRAM:401AAF8A aFeatur:        db "featur",0
DRAM:401AAF91 aFilm:          db "film",0
DRAM:401AAF96 aFootball:      db "football",0
DRAM:401AAF9F aFor_0:         db "for",0
DRAM:401AAFA3 aFrom_0:        db "from",0
DRAM:401AAFA8 aGeneralKnowledge:db "general knowledge",0
DRAM:401AAFBA aGet_0:         db "get",0
DRAM:401AAFBE aGuest:         db "guest",0
DRAM:401AAFC4 aGuests:        db "guests",0
DRAM:401AAFCB aHas:           db "has",0
DRAM:401AAFCF aHave:          db "have",0
DRAM:401AAFD4 aHeadline:      db "headline",0
DRAM:401AAFDD aHer:           db "her",0
DRAM:401AAFE1 aHis:           db "his",0
DRAM:401AAFE5 aHomeAndAbroad: db "home and abroad",0
DRAM:401AAFF5 aHost:          db "host",0
DRAM:401AAFFA aHow_0:         db "how",0
DRAM:401AAFFE aIn:            db "in",0
DRAM:401AB001 aIncluding_1:   db "including",0
DRAM:401AB00B aInternational_0:db "international",0
DRAM:401AB019 aInterview:     db "interview",0
DRAM:401AB023 aIntroduce:     db "introduce",0
DRAM:401AB02D aInvestigat:    db "investigat",0
DRAM:401AB038 aInvites:       db "invites",0
DRAM:401AB040 aIssue:         db "issue",0
DRAM:401AB046 aKnowledge:     db "knowledge",0
DRAM:401AB050 aLife:          db "life",0
DRAM:401AB055 aLive_0:        db "live",0
DRAM:401AB05A aLook_0:        db "look",0
DRAM:401AB05F aMagazine:      db "magazine",0
DRAM:401AB068 aMeets:         db "meets ",0
DRAM:401AB06F aMorning_0:     db "morning",0
DRAM:401AB077 aMorningMagazine:db "morning magazine",0
DRAM:401AB088 aMusic:         db "music",0
DRAM:401AB08E aNear:          db "near",0
DRAM:401AB093 aNetwork:       db "network",0
DRAM:401AB09B aNew_0:         db "new",0
DRAM:401AB09F aNewSeries_0:   db "new series",0
DRAM:401AB0AA aNight:         db "night",0
DRAM:401AB0B0 aOf:            db "of",0
DRAM:401AB0B3 aOn:            db "on",0
DRAM:401AB0B6 aOnight:        db "onight",0
DRAM:401AB0BD aOut:           db "out",0
DRAM:401AB0C1 aOver:          db "over",0
DRAM:401AB0C6 aPart_0:        db "part",0
DRAM:401AB0CB aPeople:        db "people",0
DRAM:401AB0D2 aPhone:         db "phone",0
DRAM:401AB0D8 aPoli:          db "poli",0
DRAM:401AB0DD aPolice:        db "police",0
DRAM:401AB0E4 aPoliticalChatShow:db "political chat show",0
DRAM:401AB0F8 aPopular:       db "popular",0
DRAM:401AB100 aPresentedBy_0: db "presented by ",0
DRAM:401AB10E aProgramm:      db "programm",0
DRAM:401AB117 aQuiz_0:        db "quiz",0
DRAM:401AB11C aReconstruction:db "reconstruction",0
DRAM:401AB12B aReport:        db "report",0
DRAM:401AB132 aReview_0:      db "review",0
DRAM:401AB139 aSchool_0:      db "school",0
DRAM:401AB140 aSeries_0:      db "series",0
DRAM:401AB147 aShort:         db "short ",0
DRAM:401AB14E aShow_0:        db "show",0
DRAM:401AB153 aSome:          db "some",0
DRAM:401AB158 aStarring:      db "starring",0
DRAM:401AB161 aStars:         db "stars",0
DRAM:401AB167 aStories:       db "stories",0
DRAM:401AB16F aStory:         db "story",0
DRAM:401AB175 aStudio:        db "studio",0
DRAM:401AB17C aSurprise:      db "surprise",0
DRAM:401AB185 aTeller:        db "teller",0
DRAM:401AB18C aThat:          db "that",0
DRAM:401AB191 aThe_0:         db "the",0
DRAM:401AB195 aTheir:         db "their",0
DRAM:401AB19B aThem:          db "them",0
DRAM:401AB1A0 aThey:          db "they",0
DRAM:401AB1A5 aThis:          db "this",0
DRAM:401AB1AA aThrough:       db "through",0
DRAM:401AB1B2 aTo:            db "to",0
DRAM:401AB1B5 aTop:           db "top",0
DRAM:401AB1B9 aTrans:         db "trans",0
DRAM:401AB1BF aUnder:         db "under",0
DRAM:401AB1C5 aUp:            db "up",0
DRAM:401AB1C8 aVery:          db "very",0
DRAM:401AB1CD aVideo:         db "video",0
DRAM:401AB1D3 aView:          db "view",0
DRAM:401AB1D8 aVintage:       db "vintage",0
DRAM:401AB1E0 aVisit:         db "visit",0
DRAM:401AB1E6 aWas:           db "was",0
DRAM:401AB1EA aWay:           db "way",0
DRAM:401AB1EE aWeek:          db "week",0
DRAM:401AB1F3 aWell:          db "well",0
DRAM:401AB1F8 aWhat:          db "what",0
DRAM:401AB1FD aWhen:          db "when",0
DRAM:401AB202 aWhich:         db "which",0
DRAM:401AB208 aWhile:         db "while",0
DRAM:401AB20E aWho:           db "who",0
DRAM:401AB212 aWill:          db "will",0
DRAM:401AB217 aWin:           db "win",0
DRAM:401AB21B aWith_0:        db "with",0
DRAM:401AB220 aWords:         db "words",0
DRAM:401AB226 aWorld_0:       db "world",0
DRAM:401AB22C aWritten:       db "written",0
DRAM:401AB234 aYear:          db "year",0
DRAM:401AB239 aYou:           db "you",0
DRAM:401AB23D                 db 0
DRAM:401AB23E                 db 0
DRAM:401AB23F                 db 0
DRAM:401AB240 huffman_table_size:db 1                 ; DATA XREF: init_or_decode_huffman+81r
DRAM:401AB240                                         ; init_or_decode_huffman+8Ar
DRAM:401AB240                                         ; init_or_decode_huffman+B5r
DRAM:401AB241                 db 0FFh
DRAM:401AB242 huffman_table_start:dw 200h
DRAM:401AB244                 dw 400h
DRAM:401AB246                 dw 600h
DRAM:401AB248                 dw 0DFFFh
DRAM:401AB24A                 dw 900h
DRAM:401AB24C                 dw 0B00h
DRAM:401AB24E                 dw 0D00h
DRAM:401AB250                 dw 0E00h
DRAM:401AB252                 dw 1000h
DRAM:401AB254                 dw 1100h
DRAM:401AB256                 dw 8BFFh
DRAM:401AB258                 dw 1200h
DRAM:401AB25A                 dw 8DFFh
DRAM:401AB25C                 dw 8CFFh
DRAM:401AB25E                 dw 1500h
DRAM:401AB260                 dw 1600h
DRAM:401AB262                 dw 93FFh
DRAM:401AB264                 dw 1900h
DRAM:401AB266                 dw 1A00h
DRAM:401AB268                 dw 1C00h
DRAM:401AB26A                 dw 1E00h
DRAM:401AB26C                 dw 1F00h
DRAM:401AB26E                 dw 2000h
DRAM:401AB270                 dw 2100h
DRAM:401AB272                 dw 2200h
DRAM:401AB274                 dw 2300h
DRAM:401AB276                 dw 2400h
DRAM:401AB278                 dw 88FFh
DRAM:401AB27A                 dw 2600h
DRAM:401AB27C                 dw 0D1FFh
DRAM:401AB27E                 dw 2800h
DRAM:401AB280                 dw 9DFFh
DRAM:401AB282                 dw 0D3FFh
DRAM:401AB284                 dw 89FFh
DRAM:401AB286                 dw 2D00h
DRAM:401AB288                 dw 94FFh
DRAM:401AB28A                 dw 3000h
DRAM:401AB28C                 dw 3100h
DRAM:401AB28E                 dw 0F6FFh
DRAM:401AB290                 dw 3300h
DRAM:401AB292                 dw 3500h
DRAM:401AB294                 dw 0BEFFh
DRAM:401AB296                 dw 0A8FFh
DRAM:401AB298                 dw 3900h
DRAM:401AB29A                 dw 3A00h
DRAM:401AB29C                 dw 3B00h
DRAM:401AB29E                 dw 0D8FFh
DRAM:401AB2A0                 dw 0BCFFh
DRAM:401AB2A2                 dw 0BDFFh
DRAM:401AB2A4                 dw 3F00h
DRAM:401AB2A6                 dw 0ADFFh
DRAM:401AB2A8                 dw 4100h
DRAM:401AB2AA                 dw 0CFFFh
DRAM:401AB2AC                 dw 4400h
DRAM:401AB2AE                 dw 4500h
DRAM:401AB2B0                 dw 4600h
DRAM:401AB2B2                 dw 4800h
DRAM:401AB2B4                 dw 0B9FFh
DRAM:401AB2B6                 dw 4A00h
DRAM:401AB2B8                 dw 0B8FFh
DRAM:401AB2BA                 dw 0B3FFh
DRAM:401AB2BC                 dw 4D00h
DRAM:401AB2BE                 dw 0B0FFh
DRAM:401AB2C0                 dw 4E00h
DRAM:401AB2C2                 dw 4F00h
DRAM:401AB2C4                 dw 0CAFFh
DRAM:401AB2C6                 dw 0FCFEh
DRAM:401AB2C8                 dw 5200h
DRAM:401AB2CA                 dw 0BAFFh
DRAM:401AB2CC                 dw 0F9FEh
DRAM:401AB2CE                 dw 85FFh
DRAM:401AB2D0                 dw 5600h
DRAM:401AB2D2                 dw 0D6FFh
DRAM:401AB2D4                 dw 5900h
DRAM:401AB2D6                 dw 0CDFFh
DRAM:401AB2D8                 dw 0C8FFh
DRAM:401AB2DA                 dw 5B00h
DRAM:401AB2DC                 dw 5C00h
DRAM:401AB2DE                 dw 0CBFFh
DRAM:401AB2E0                 dw 5E00h
DRAM:401AB2E2                 dw 9FFFh
DRAM:401AB2E4                 dw 0C5FFh
DRAM:401AB2E6                 dw 6100h
DRAM:401AB2E8                 dw 95FFh
DRAM:401AB2EA                 dw 0FEh
DRAM:401AB2EC                 dw 0A6FFh
DRAM:401AB2EE                 dw 6500h
DRAM:401AB2F0                 dw 6600h
DRAM:401AB2F2                 dw 0C7FFh
DRAM:401AB2F4                 dw 6900h
DRAM:401AB2F6                 dw 6A00h
DRAM:401AB2F8                 dw 6B00h
DRAM:401AB2FA                 dw 0DEFFh
DRAM:401AB2FC                 dw 0C4FFh
DRAM:401AB2FE                 dw 6F00h
DRAM:401AB300                 dw 7000h
DRAM:401AB302                 dw 7200h
DRAM:401AB304                 dw 7400h
DRAM:401AB306                 dw 7500h
DRAM:401AB308                 dw 7700h
DRAM:401AB30A                 dw 7900h
DRAM:401AB30C                 dw 7A00h
DRAM:401AB30E                 dw 0DEFEh
DRAM:401AB310                 dw 0D5FFh
DRAM:401AB312                 dw 7E00h
DRAM:401AB314                 dw 8000h
DRAM:401AB316                 dw 1FEh
DRAM:401AB318                 dw 8300h
DRAM:401AB31A                 dw 8500h
DRAM:401AB31C                 dw 0EAFEh
DRAM:401AB31E                 dw 8800h
DRAM:401AB320                 dw 0A5FFh
DRAM:401AB322                 dw 0C1FFh
DRAM:401AB324                 dw 8B00h
DRAM:401AB326                 dw 0E8FEh
DRAM:401AB328                 dw 8D00h
DRAM:401AB32A                 dw 0B9FEh
DRAM:401AB32C                 dw 9000h
DRAM:401AB32E                 dw 0EBFEh
DRAM:401AB330                 dw 9200h
DRAM:401AB332                 dw 0D2FEh
DRAM:401AB334                 dw 0E9FEh
DRAM:401AB336                 dw 9500h
DRAM:401AB338                 dw 0CDFEh
DRAM:401AB33A                 dw 0C0FEh
DRAM:401AB33C                 dw 9600h
DRAM:401AB33E                 dw 0C6FEh
DRAM:401AB340                 dw 0D3FEh
DRAM:401AB342                 dw 0ECFEh
DRAM:401AB344                 dw 0D1FEh
DRAM:401AB346                 dw 0C4FEh
DRAM:401AB348                 dw 0C8FEh
DRAM:401AB34A                 dw 0E7FEh
DRAM:401AB34C                 dw 0F5FEh
DRAM:401AB34E                 dw 0D9FEh
DRAM:401AB350                 dw 9A00h
DRAM:401AB352                 dw 0C2FEh
DRAM:401AB354                 dw 0F2FEh
DRAM:401AB356                 dw 0C9FEh
DRAM:401AB358                 dw 0B8FEh
DRAM:401AB35A                 dw 0BFFEh
DRAM:401AB35C                 dw 0CCFEh
DRAM:401AB35E                 dw 0FEFEh
DRAM:401AB360                 dw 9E00h
DRAM:401AB362                 dw 0F0FEh
DRAM:401AB364                 dw 0B7FEh
DRAM:401AB366                 dw 0EEFEh
DRAM:401AB368                 dw 0FFFEh
DRAM:401AB36A                 dw 0D7FEh
DRAM:401AB36C                 dw 0E5FEh
DRAM:401AB36E                 dw 0BBFEh
DRAM:401AB370                 dw 0D0FEh
DRAM:401AB372                 dw 0BCFEh
DRAM:401AB374                 dw 0D0FFh
DRAM:401AB376                 dw 0CBFEh
DRAM:401AB378                 dw 0E0FEh
DRAM:401AB37A                 dw 0DFFEh
DRAM:401AB37C                 dw 0B6FEh
DRAM:401AB37E                 dw 0D6FEh
DRAM:401AB380                 dw 0A200h
DRAM:401AB382                 dw 0F8FEh
DRAM:401AB384                 dw 0A300h
DRAM:401AB386                 dw 0A400h
DRAM:401AB388                 dw 0A2FFh
DRAM:401AB38A                 dw 0A500h
DRAM:401AB38C                 dw 0A600h
DRAM:401AB38E                 dw 0A800h
DRAM:401AB390                 dw 0AA00h
DRAM:401AB392                 dw 0AC00h
DRAM:401AB394                 dw 0AE00h
DRAM:401AB396                 dw 0B000h
DRAM:401AB398                 dw 0B200h
DRAM:401AB39A                 dw 0B400h
DRAM:401AB39C                 dw 0B600h
DRAM:401AB39E                 dw 0B800h
DRAM:401AB3A0                 dw 0BA00h
DRAM:401AB3A2                 dw 0BC00h
DRAM:401AB3A4                 dw 0BE00h
DRAM:401AB3A6                 dw 0C000h
DRAM:401AB3A8                 dw 0C200h
DRAM:401AB3AA                 dw 0C400h
DRAM:401AB3AC                 dw 0C600h
DRAM:401AB3AE                 dw 0C800h
DRAM:401AB3B0                 dw 0CA00h
DRAM:401AB3B2                 dw 0CC00h
DRAM:401AB3B4                 dw 0CE00h
DRAM:401AB3B6                 dw 0D000h
DRAM:401AB3B8                 dw 0D200h
DRAM:401AB3BA                 dw 0D400h
DRAM:401AB3BC                 dw 0D600h
DRAM:401AB3BE                 dw 0D800h
DRAM:401AB3C0                 dw 0DA00h
DRAM:401AB3C2                 dw 0DC00h
DRAM:401AB3C4                 dw 0DE00h
DRAM:401AB3C6                 dw 0E000h
DRAM:401AB3C8                 dw 0E200h
DRAM:401AB3CA                 dw 0E400h
DRAM:401AB3CC                 dw 0E600h
DRAM:401AB3CE                 dw 0E800h
DRAM:401AB3D0                 dw 0EA00h
DRAM:401AB3D2                 dw 0EC00h
DRAM:401AB3D4                 dw 0EE00h
DRAM:401AB3D6                 dw 0F000h
DRAM:401AB3D8                 dw 0F200h
DRAM:401AB3DA                 dw 0F400h
DRAM:401AB3DC                 dw 0F600h
DRAM:401AB3DE                 dw 0F800h
DRAM:401AB3E0                 dw 0FA00h
DRAM:401AB3E2                 dw 0FC00h
DRAM:401AB3E4                 dw 0FE00h
DRAM:401AB3E6                 dw 1
DRAM:401AB3E8                 dw 201h
DRAM:401AB3EA                 dw 401h
DRAM:401AB3EC                 dw 601h
DRAM:401AB3EE                 dw 801h
DRAM:401AB3F0                 dw 0A01h
DRAM:401AB3F2                 dw 0C01h
DRAM:401AB3F4                 dw 0E01h
DRAM:401AB3F6                 dw 1001h
DRAM:401AB3F8                 dw 1201h
DRAM:401AB3FA                 dw 1401h
DRAM:401AB3FC                 dw 1601h
DRAM:401AB3FE                 dw 1801h
DRAM:401AB400                 dw 1A01h
DRAM:401AB402                 dw 1C01h
DRAM:401AB404                 dw 1E01h
DRAM:401AB406                 dw 2001h
DRAM:401AB408                 dw 2201h
DRAM:401AB40A                 dw 2401h
DRAM:401AB40C                 dw 2601h
DRAM:401AB40E                 dw 2801h
DRAM:401AB410                 dw 2A01h
DRAM:401AB412                 dw 2C01h
DRAM:401AB414                 dw 2E01h
DRAM:401AB416                 dw 3001h
DRAM:401AB418                 dw 3201h
DRAM:401AB41A                 dw 3401h
DRAM:401AB41C                 dw 3601h
DRAM:401AB41E                 dw 3801h
DRAM:401AB420                 dw 3A01h
DRAM:401AB422                 dw 3C01h
DRAM:401AB424                 dw 3E01h
DRAM:401AB426                 dw 4001h
DRAM:401AB428                 dw 4201h
DRAM:401AB42A                 dw 4401h
DRAM:401AB42C                 dw 4601h
DRAM:401AB42E                 dw 4801h
DRAM:401AB430                 dw 4A01h
DRAM:401AB432                 dw 4C01h
DRAM:401AB434                 dw 4E01h
DRAM:401AB436                 dw 5001h
DRAM:401AB438                 dw 5201h
DRAM:401AB43A                 dw 5401h
DRAM:401AB43C                 dw 5601h
DRAM:401AB43E                 dw 5801h
DRAM:401AB440                 dw 5A01h
DRAM:401AB442                 dw 5C01h
DRAM:401AB444                 dw 5E01h
DRAM:401AB446                 dw 6001h
DRAM:401AB448                 dw 6201h
DRAM:401AB44A                 dw 6401h
DRAM:401AB44C                 dw 6601h
DRAM:401AB44E                 dw 6801h
DRAM:401AB450                 dw 6A01h
DRAM:401AB452                 dw 6C01h
DRAM:401AB454                 dw 6E01h
DRAM:401AB456                 dw 7001h
DRAM:401AB458                 dw 7201h
DRAM:401AB45A                 dw 7401h
DRAM:401AB45C                 dw 7601h
DRAM:401AB45E                 dw 7801h
DRAM:401AB460                 dw 7A01h
DRAM:401AB462                 dw 7C01h
DRAM:401AB464                 dw 7E01h
DRAM:401AB466                 dw 8001h
DRAM:401AB468                 dw 8201h
DRAM:401AB46A                 dw 8401h
DRAM:401AB46C                 dw 8601h
DRAM:401AB46E                 dw 8801h
DRAM:401AB470                 dw 8A01h
DRAM:401AB472                 dw 8C01h
DRAM:401AB474                 dw 8E01h
DRAM:401AB476                 dw 9001h
DRAM:401AB478                 dw 9201h
DRAM:401AB47A                 dw 9401h
DRAM:401AB47C                 dw 9601h
DRAM:401AB47E                 dw 9801h
DRAM:401AB480                 dw 9A01h
DRAM:401AB482                 dw 9C01h
DRAM:401AB484                 dw 9E01h
DRAM:401AB486                 dw 0A001h
DRAM:401AB488                 dw 0A201h
DRAM:401AB48A                 dw 0A401h
DRAM:401AB48C                 dw 0A601h
DRAM:401AB48E                 dw 0A801h
DRAM:401AB490                 dw 0AA01h
DRAM:401AB492                 dw 0AC01h
DRAM:401AB494                 dw 0AE01h
DRAM:401AB496                 dw 0B001h
DRAM:401AB498                 dw 0B201h
DRAM:401AB49A                 dw 0B401h
DRAM:401AB49C                 dw 0B601h
DRAM:401AB49E                 dw 0B801h
DRAM:401AB4A0                 dw 0BA01h
DRAM:401AB4A2                 dw 0BC01h
DRAM:401AB4A4                 dw 0BE01h
DRAM:401AB4A6                 dw 0C001h
DRAM:401AB4A8                 dw 0C201h
DRAM:401AB4AA                 dw 0C401h
DRAM:401AB4AC                 dw 0C601h
DRAM:401AB4AE                 dw 0C801h
DRAM:401AB4B0                 dw 0CA01h
DRAM:401AB4B2                 dw 0CC01h
DRAM:401AB4B4                 dw 0CE01h
DRAM:401AB4B6                 dw 0D001h
DRAM:401AB4B8                 dw 0D201h
DRAM:401AB4BA                 dw 0D401h
DRAM:401AB4BC                 dw 0D601h
DRAM:401AB4BE                 dw 0D801h
DRAM:401AB4C0                 dw 0DA01h
DRAM:401AB4C2                 dw 0DC01h
DRAM:401AB4C4                 dw 0DE01h
DRAM:401AB4C6                 dw 0E001h
DRAM:401AB4C8                 dw 0E201h
DRAM:401AB4CA                 dw 0E401h
DRAM:401AB4CC                 dw 0E601h
DRAM:401AB4CE                 dw 0E801h
DRAM:401AB4D0                 dw 0EA01h
DRAM:401AB4D2                 dw 0EC01h
DRAM:401AB4D4                 dw 0EE01h
DRAM:401AB4D6                 dw 0F001h
DRAM:401AB4D8                 dw 0F201h
DRAM:401AB4DA                 dw 0F401h
DRAM:401AB4DC                 dw 0F601h
DRAM:401AB4DE                 dw 0F801h
DRAM:401AB4E0                 dw 0FA01h
DRAM:401AB4E2                 dw 0FC01h
DRAM:401AB4E4                 dw 0FE01h
DRAM:401AB4E6                 dw 3FEh
DRAM:401AB4E8                 dw 5FEh
DRAM:401AB4EA                 dw 7FEh
DRAM:401AB4EC                 dw 9FEh
DRAM:401AB4EE                 dw 0BFEh
DRAM:401AB4F0                 dw 0DFEh
DRAM:401AB4F2                 dw 0FFEh
DRAM:401AB4F4                 dw 11FEh
DRAM:401AB4F6                 dw 13FEh
DRAM:401AB4F8                 dw 15FEh
DRAM:401AB4FA                 dw 17FEh
DRAM:401AB4FC                 dw 19FEh
DRAM:401AB4FE                 dw 1BFEh
DRAM:401AB500                 dw 1DFEh
DRAM:401AB502                 dw 1FFEh
DRAM:401AB504                 dw 21FEh
DRAM:401AB506                 dw 23FEh
DRAM:401AB508                 dw 25FEh
DRAM:401AB50A                 dw 27FEh
DRAM:401AB50C                 dw 29FEh
DRAM:401AB50E                 dw 2BFEh
DRAM:401AB510                 dw 2DFEh
DRAM:401AB512                 dw 2FFEh
DRAM:401AB514                 dw 31FEh
DRAM:401AB516                 dw 33FEh
DRAM:401AB518                 dw 35FEh
DRAM:401AB51A                 dw 37FEh
DRAM:401AB51C                 dw 39FEh
DRAM:401AB51E                 dw 3BFEh
DRAM:401AB520                 dw 3DFEh
DRAM:401AB522                 dw 3FFEh
DRAM:401AB524                 dw 41FEh
DRAM:401AB526                 dw 43FEh
DRAM:401AB528                 dw 45FEh
DRAM:401AB52A                 dw 47FEh
DRAM:401AB52C                 dw 49FEh
DRAM:401AB52E                 dw 4BFEh
DRAM:401AB530                 dw 4DFEh
DRAM:401AB532                 dw 4FFEh
DRAM:401AB534                 dw 51FEh
DRAM:401AB536                 dw 53FEh
DRAM:401AB538                 dw 55FEh
DRAM:401AB53A                 dw 57FEh
DRAM:401AB53C                 dw 59FEh
DRAM:401AB53E                 dw 5BFEh
DRAM:401AB540                 dw 5DFEh
DRAM:401AB542                 dw 5FFEh
DRAM:401AB544                 dw 61FEh
DRAM:401AB546                 dw 63FEh
DRAM:401AB548                 dw 65FEh
DRAM:401AB54A                 dw 67FEh
DRAM:401AB54C                 dw 69FEh
DRAM:401AB54E                 dw 6BFEh
DRAM:401AB550                 dw 6DFEh
DRAM:401AB552                 dw 6FFEh
DRAM:401AB554                 dw 71FEh
DRAM:401AB556                 dw 73FEh
DRAM:401AB558                 dw 75FEh
DRAM:401AB55A                 dw 77FEh
DRAM:401AB55C                 dw 79FEh
DRAM:401AB55E                 dw 7BFEh
DRAM:401AB560                 dw 7DFEh
DRAM:401AB562                 dw 7FFEh
DRAM:401AB564                 dw 81FEh
DRAM:401AB566                 dw 83FEh
DRAM:401AB568                 dw 85FEh
DRAM:401AB56A                 dw 87FEh
DRAM:401AB56C                 dw 89FEh
DRAM:401AB56E                 dw 8BFEh
DRAM:401AB570                 dw 8DFEh
DRAM:401AB572                 dw 8FFEh
DRAM:401AB574                 dw 91FEh
DRAM:401AB576                 dw 93FEh
DRAM:401AB578                 dw 95FEh
DRAM:401AB57A                 dw 97FEh
DRAM:401AB57C                 dw 99FEh
DRAM:401AB57E                 dw 9BFEh
DRAM:401AB580                 dw 9DFEh
DRAM:401AB582                 dw 9FFEh
DRAM:401AB584                 dw 0A1FEh
DRAM:401AB586                 dw 0A3FEh
DRAM:401AB588                 dw 0A5FEh
DRAM:401AB58A                 dw 0A7FEh
DRAM:401AB58C                 dw 0A9FEh
DRAM:401AB58E                 dw 0ABFEh
DRAM:401AB590                 dw 0ADFEh
DRAM:401AB592                 dw 0AFFEh
DRAM:401AB594                 dw 0B1FEh
DRAM:401AB596                 dw 1FFh
DRAM:401AB598                 dw 3FFh
DRAM:401AB59A                 dw 5FFh
DRAM:401AB59C                 dw 7FFh
DRAM:401AB59E                 dw 9FFh
DRAM:401AB5A0                 dw 0BFFh
DRAM:401AB5A2                 dw 0DFFh
DRAM:401AB5A4                 dw 0FFFh
DRAM:401AB5A6                 dw 11FFh
DRAM:401AB5A8                 dw 13FFh
DRAM:401AB5AA                 dw 15FFh
DRAM:401AB5AC                 dw 17FFh
DRAM:401AB5AE                 dw 19FFh
DRAM:401AB5B0                 dw 1BFFh
DRAM:401AB5B2                 dw 1DFFh
DRAM:401AB5B4                 dw 1FFFh
DRAM:401AB5B6                 dw 21FFh
DRAM:401AB5B8                 dw 23FFh
DRAM:401AB5BA                 dw 25FFh
DRAM:401AB5BC                 dw 27FFh
DRAM:401AB5BE                 dw 29FFh
DRAM:401AB5C0                 dw 2BFFh
DRAM:401AB5C2                 dw 2DFFh
DRAM:401AB5C4                 dw 2FFFh
DRAM:401AB5C6                 dw 31FFh
DRAM:401AB5C8                 dw 33FFh
DRAM:401AB5CA                 dw 35FFh
DRAM:401AB5CC                 dw 37FFh
DRAM:401AB5CE                 dw 39FFh
DRAM:401AB5D0                 dw 3BFFh
DRAM:401AB5D2                 dw 3DFFh
DRAM:401AB5D4                 dw 3FFFh
DRAM:401AB5D6                 dw 41FFh
DRAM:401AB5D8                 dw 43FFh
DRAM:401AB5DA                 dw 45FFh
DRAM:401AB5DC                 dw 47FFh
DRAM:401AB5DE                 dw 49FFh
DRAM:401AB5E0                 dw 4BFFh
DRAM:401AB5E2                 dw 4DFFh
DRAM:401AB5E4                 dw 4FFFh
DRAM:401AB5E6                 dw 51FFh
DRAM:401AB5E8                 dw 53FFh
DRAM:401AB5EA                 dw 55FFh
DRAM:401AB5EC                 dw 57FFh
DRAM:401AB5EE                 dw 59FFh
DRAM:401AB5F0                 dw 5BFFh
DRAM:401AB5F2                 dw 5DFFh
DRAM:401AB5F4                 dw 5FFFh
DRAM:401AB5F6                 dw 61FFh
DRAM:401AB5F8                 dw 64FFh
DRAM:401AB5FA                 dw 66FFh
DRAM:401AB5FC                 dw 68FFh
DRAM:401AB5FE                 dw 6AFFh
DRAM:401AB600                 dw 6CFFh
DRAM:401AB602                 dw 6EFFh
DRAM:401AB604                 dw 70FFh
DRAM:401AB606                 dw 72FFh
DRAM:401AB608                 dw 74FFh
DRAM:401AB60A                 dw 76FFh
DRAM:401AB60C                 dw 78FFh
DRAM:401AB60E                 dw 7AFFh
DRAM:401AB610                 dw 7CFFh
DRAM:401AB612                 dw 7EFFh
DRAM:401AB614                 dw 80FFh
DRAM:401AB616                 dw 82FFh
DRAM:401AB618                 dw 84FFh
DRAM:401AB61A                 dw 0A1FFh
DRAM:401AB61C                 dw 0C2FFh
DRAM:401AB61E                 dw 0DAFFh
DRAM:401AB620                 dw 0DDFFh
DRAM:401AB622                 dw 0E1FFh
DRAM:401AB624                 dw 0E3FFh
DRAM:401AB626                 dw 0E5FFh
DRAM:401AB628                 dw 0E7FFh
DRAM:401AB62A                 dw 0E9FFh
DRAM:401AB62C                 dw 0EBFFh
DRAM:401AB62E                 dw 0EDFFh
DRAM:401AB630                 dw 0EFFFh
DRAM:401AB632                 dw 0F1FFh
DRAM:401AB634                 dw 0F3FFh
DRAM:401AB636                 dw 0F7FFh
DRAM:401AB638                 dw 0F9FFh
DRAM:401AB63A                 dw 0FBFFh
DRAM:401AB63C                 dw 0FDFFh
DRAM:401AB63E                 dw 0FFFFh
DRAM:401AB640                 dw 100h
DRAM:401AB642                 dw 300h
DRAM:401AB644                 dw 500h
DRAM:401AB646                 dw 700h
DRAM:401AB648                 dw 800h
DRAM:401AB64A                 dw 0A00h
DRAM:401AB64C                 dw 0C00h
DRAM:401AB64E                 dw 9AFFh
DRAM:401AB650                 dw 0F00h
DRAM:401AB652                 dw 9EFFh
DRAM:401AB654                 dw 91FFh
DRAM:401AB656                 dw 96FFh
DRAM:401AB658                 dw 90FFh
DRAM:401AB65A                 dw 1300h
DRAM:401AB65C                 dw 1400h
DRAM:401AB65E                 dw 97FFh
DRAM:401AB660                 dw 1700h
DRAM:401AB662                 dw 1800h
DRAM:401AB664                 dw 9BFFh
DRAM:401AB666                 dw 1B00h
DRAM:401AB668                 dw 1D00h
DRAM:401AB66A                 dw 9CFFh
DRAM:401AB66C                 dw 8AFFh
DRAM:401AB66E                 dw 92FFh
DRAM:401AB670                 dw 98FFh
DRAM:401AB672                 dw 99FFh
DRAM:401AB674                 dw 86FFh
DRAM:401AB676                 dw 8FFFh
DRAM:401AB678                 dw 2500h
DRAM:401AB67A                 dw 0F5FFh
DRAM:401AB67C                 dw 2700h
DRAM:401AB67E                 dw 2900h
DRAM:401AB680                 dw 2A00h
DRAM:401AB682                 dw 2B00h
DRAM:401AB684                 dw 2C00h
DRAM:401AB686                 dw 2E00h
DRAM:401AB688                 dw 2F00h
DRAM:401AB68A                 dw 0ABFFh
DRAM:401AB68C                 dw 0ACFFh
DRAM:401AB68E                 dw 3200h
DRAM:401AB690                 dw 3400h
DRAM:401AB692                 dw 3600h
DRAM:401AB694                 dw 3700h
DRAM:401AB696                 dw 3800h
DRAM:401AB698                 dw 0D2FFh
DRAM:401AB69A                 dw 0B2FFh
DRAM:401AB69C                 dw 3C00h
DRAM:401AB69E                 dw 3D00h
DRAM:401AB6A0                 dw 3E00h
DRAM:401AB6A2                 dw 0AFFFh
DRAM:401AB6A4                 dw 0B1FFh
DRAM:401AB6A6                 dw 4000h
DRAM:401AB6A8                 dw 4200h
DRAM:401AB6AA                 dw 4300h
DRAM:401AB6AC                 dw 0BBFFh
DRAM:401AB6AE                 dw 0CEFFh
DRAM:401AB6B0                 dw 4700h
DRAM:401AB6B2                 dw 0B7FFh
DRAM:401AB6B4                 dw 4900h
DRAM:401AB6B6                 dw 4B00h
DRAM:401AB6B8                 dw 4C00h
DRAM:401AB6BA                 dw 0FBFEh
DRAM:401AB6BC                 dw 0FAFEh
DRAM:401AB6BE                 dw 0B5FFh
DRAM:401AB6C0                 dw 87FFh
DRAM:401AB6C2                 dw 5000h
DRAM:401AB6C4                 dw 0B4FFh
DRAM:401AB6C6                 dw 5100h
DRAM:401AB6C8                 dw 0B6FFh
DRAM:401AB6CA                 dw 5300h
DRAM:401AB6CC                 dw 5400h
DRAM:401AB6CE                 dw 5500h
DRAM:401AB6D0                 dw 5700h
DRAM:401AB6D2                 dw 5800h
DRAM:401AB6D4                 dw 0C9FFh
DRAM:401AB6D6                 dw 5A00h
DRAM:401AB6D8                 dw 0CCFFh
DRAM:401AB6DA                 dw 0C6FFh
DRAM:401AB6DC                 dw 5D00h
DRAM:401AB6DE                 dw 0A9FFh
DRAM:401AB6E0                 dw 5F00h
DRAM:401AB6E2                 dw 6000h
DRAM:401AB6E4                 dw 0D7FFh
DRAM:401AB6E6                 dw 6200h
DRAM:401AB6E8                 dw 6300h
DRAM:401AB6EA                 dw 6400h
DRAM:401AB6EC                 dw 0AAFFh
DRAM:401AB6EE                 dw 8EFFh
DRAM:401AB6F0                 dw 6700h
DRAM:401AB6F2                 dw 6800h
DRAM:401AB6F4                 dw 0BEFEh
DRAM:401AB6F6                 dw 0D9FFh
DRAM:401AB6F8                 dw 6C00h
DRAM:401AB6FA                 dw 6D00h
DRAM:401AB6FC                 dw 6E00h
DRAM:401AB6FE                 dw 0C0FFh
DRAM:401AB700                 dw 7100h
DRAM:401AB702                 dw 7300h
DRAM:401AB704                 dw 0D5FEh
DRAM:401AB706                 dw 7600h
DRAM:401AB708                 dw 7800h
DRAM:401AB70A                 dw 0B3FEh
DRAM:401AB70C                 dw 7B00h
DRAM:401AB70E                 dw 7C00h
DRAM:401AB710                 dw 7D00h
DRAM:401AB712                 dw 7F00h
DRAM:401AB714                 dw 8100h
DRAM:401AB716                 dw 8200h
DRAM:401AB718                 dw 8400h
DRAM:401AB71A                 dw 8600h
DRAM:401AB71C                 dw 8700h
DRAM:401AB71E                 dw 8900h
DRAM:401AB720                 dw 8A00h
DRAM:401AB722                 dw 0C3FFh
DRAM:401AB724                 dw 0FDFEh
DRAM:401AB726                 dw 8C00h
DRAM:401AB728                 dw 8E00h
DRAM:401AB72A                 dw 8F00h
DRAM:401AB72C                 dw 9100h
DRAM:401AB72E                 dw 0B2FEh
DRAM:401AB730                 dw 9300h
DRAM:401AB732                 dw 0E6FEh
DRAM:401AB734                 dw 9400h
DRAM:401AB736                 dw 0C3FEh
DRAM:401AB738                 dw 0EDFEh
DRAM:401AB73A                 dw 0AEFFh
DRAM:401AB73C                 dw 0B5FEh
DRAM:401AB73E                 dw 0CAFEh
DRAM:401AB740                 dw 0DCFEh
DRAM:401AB742                 dw 9700h
DRAM:401AB744                 dw 9800h
DRAM:401AB746                 dw 0B4FEh
DRAM:401AB748                 dw 0DDFEh
DRAM:401AB74A                 dw 0F3FEh
DRAM:401AB74C                 dw 9900h
DRAM:401AB74E                 dw 0E3FEh
DRAM:401AB750                 dw 9B00h
DRAM:401AB752                 dw 0EFFEh
DRAM:401AB754                 dw 9C00h
DRAM:401AB756                 dw 0D4FEh
DRAM:401AB758                 dw 0BDFEh
DRAM:401AB75A                 dw 0C5FEh
DRAM:401AB75C                 dw 0E1FEh
DRAM:401AB75E                 dw 9D00h
DRAM:401AB760                 dw 0E2FEh
DRAM:401AB762                 dw 0F6FEh
DRAM:401AB764                 dw 0D8FEh
DRAM:401AB766                 dw 0F1FEh
DRAM:401AB768                 dw 9F00h
DRAM:401AB76A                 dw 0E4FEh
DRAM:401AB76C                 dw 0BAFEh
DRAM:401AB76E                 dw 0CFFEh
DRAM:401AB770                 dw 0A000h
DRAM:401AB772                 dw 0DAFEh
DRAM:401AB774                 dw 0C1FEh
DRAM:401AB776                 dw 0CEFEh
DRAM:401AB778                 dw 0F4FEh
DRAM:401AB77A                 dw 0A100h
DRAM:401AB77C                 dw 0C7FEh
DRAM:401AB77E                 dw 0F7FEh
DRAM:401AB780                 dw 0DBFEh
DRAM:401AB782                 dw 0A7FFh
DRAM:401AB784                 dw 0DBFFh
DRAM:401AB786                 dw 63FFh
DRAM:401AB788                 dw 0A4FFh
DRAM:401AB78A                 dw 0BFFFh
DRAM:401AB78C                 dw 0A700h
DRAM:401AB78E                 dw 0A900h
DRAM:401AB790                 dw 0AB00h
DRAM:401AB792                 dw 0AD00h
DRAM:401AB794                 dw 0AF00h
DRAM:401AB796                 dw 0B100h
DRAM:401AB798                 dw 0B300h
DRAM:401AB79A                 dw 0B500h
DRAM:401AB79C                 dw 0B700h
DRAM:401AB79E                 dw 0B900h
DRAM:401AB7A0                 dw 0BB00h
DRAM:401AB7A2                 dw 0BD00h
DRAM:401AB7A4                 dw 0BF00h
DRAM:401AB7A6                 dw 0C100h
DRAM:401AB7A8                 dw 0C300h
DRAM:401AB7AA                 dw 0C500h
DRAM:401AB7AC                 dw 0C700h
DRAM:401AB7AE                 dw 0C900h
DRAM:401AB7B0                 dw 0CB00h
DRAM:401AB7B2                 dw 0CD00h
DRAM:401AB7B4                 dw 0CF00h
DRAM:401AB7B6                 dw 0D100h
DRAM:401AB7B8                 dw 0D300h
DRAM:401AB7BA                 dw 0D500h
DRAM:401AB7BC                 dw 0D700h
DRAM:401AB7BE                 dw 0D900h
DRAM:401AB7C0                 dw 0DB00h
DRAM:401AB7C2                 dw 0DD00h
DRAM:401AB7C4                 dw 0DF00h
DRAM:401AB7C6                 dw 0E100h
DRAM:401AB7C8                 dw 0E300h
DRAM:401AB7CA                 dw 0E500h
DRAM:401AB7CC                 dw 0E700h
DRAM:401AB7CE                 dw 0E900h
DRAM:401AB7D0                 dw 0EB00h
DRAM:401AB7D2                 dw 0ED00h
DRAM:401AB7D4                 dw 0EF00h
DRAM:401AB7D6                 dw 0F100h
DRAM:401AB7D8                 dw 0F300h
DRAM:401AB7DA                 dw 0F500h
DRAM:401AB7DC                 dw 0F700h
DRAM:401AB7DE                 dw 0F900h
DRAM:401AB7E0                 dw 0FB00h
DRAM:401AB7E2                 dw 0FD00h
DRAM:401AB7E4                 dw 0FF00h
DRAM:401AB7E6                 dw 101h
DRAM:401AB7E8                 dw 301h
DRAM:401AB7EA                 dw 501h
DRAM:401AB7EC                 dw 701h
DRAM:401AB7EE                 dw 901h
DRAM:401AB7F0                 dw 0B01h
DRAM:401AB7F2                 dw 0D01h
DRAM:401AB7F4                 dw 0F01h
DRAM:401AB7F6                 dw 1101h
DRAM:401AB7F8                 dw 1301h
DRAM:401AB7FA                 dw 1501h
DRAM:401AB7FC                 dw 1701h
DRAM:401AB7FE                 dw 1901h
DRAM:401AB800                 dw 1B01h
DRAM:401AB802                 dw 1D01h
DRAM:401AB804                 dw 1F01h
DRAM:401AB806                 dw 2101h
DRAM:401AB808                 dw 2301h
DRAM:401AB80A                 dw 2501h
DRAM:401AB80C                 dw 2701h
DRAM:401AB80E                 dw 2901h
DRAM:401AB810                 dw 2B01h
DRAM:401AB812                 dw 2D01h
DRAM:401AB814                 dw 2F01h
DRAM:401AB816                 dw 3101h
DRAM:401AB818                 dw 3301h
DRAM:401AB81A                 dw 3501h
DRAM:401AB81C                 dw 3701h
DRAM:401AB81E                 dw 3901h
DRAM:401AB820                 dw 3B01h
DRAM:401AB822                 dw 3D01h
DRAM:401AB824                 dw 3F01h
DRAM:401AB826                 dw 4101h
DRAM:401AB828                 dw 4301h
DRAM:401AB82A                 dw 4501h
DRAM:401AB82C                 dw 4701h
DRAM:401AB82E                 dw 4901h
DRAM:401AB830                 dw 4B01h
DRAM:401AB832                 dw 4D01h
DRAM:401AB834                 dw 4F01h
DRAM:401AB836                 dw 5101h
DRAM:401AB838                 dw 5301h
DRAM:401AB83A                 dw 5501h
DRAM:401AB83C                 dw 5701h
DRAM:401AB83E                 dw 5901h
DRAM:401AB840                 dw 5B01h
DRAM:401AB842                 dw 5D01h
DRAM:401AB844                 dw 5F01h
DRAM:401AB846                 dw 6101h
DRAM:401AB848                 dw 6301h
DRAM:401AB84A                 dw 6501h
DRAM:401AB84C                 dw 6701h
DRAM:401AB84E                 dw 6901h
DRAM:401AB850                 dw 6B01h
DRAM:401AB852                 dw 6D01h
DRAM:401AB854                 dw 6F01h
DRAM:401AB856                 dw 7101h
DRAM:401AB858                 dw 7301h
DRAM:401AB85A                 dw 7501h
DRAM:401AB85C                 dw 7701h
DRAM:401AB85E                 dw 7901h
DRAM:401AB860                 dw 7B01h
DRAM:401AB862                 dw 7D01h
DRAM:401AB864                 dw 7F01h
DRAM:401AB866                 dw 8101h
DRAM:401AB868                 dw 8301h
DRAM:401AB86A                 dw 8501h
DRAM:401AB86C                 dw 8701h
DRAM:401AB86E                 dw 8901h
DRAM:401AB870                 dw 8B01h
DRAM:401AB872                 dw 8D01h
DRAM:401AB874                 dw 8F01h
DRAM:401AB876                 dw 9101h
DRAM:401AB878                 dw 9301h
DRAM:401AB87A                 dw 9501h
DRAM:401AB87C                 dw 9701h
DRAM:401AB87E                 dw 9901h
DRAM:401AB880                 dw 9B01h
DRAM:401AB882                 dw 9D01h
DRAM:401AB884                 dw 9F01h
DRAM:401AB886                 dw 0A101h
DRAM:401AB888                 dw 0A301h
DRAM:401AB88A                 dw 0A501h
DRAM:401AB88C                 dw 0A701h
DRAM:401AB88E                 dw 0A901h
DRAM:401AB890                 dw 0AB01h
DRAM:401AB892                 dw 0AD01h
DRAM:401AB894                 dw 0AF01h
DRAM:401AB896                 dw 0B101h
DRAM:401AB898                 dw 0B301h
DRAM:401AB89A                 dw 0B501h
DRAM:401AB89C                 dw 0B701h
DRAM:401AB89E                 dw 0B901h
DRAM:401AB8A0                 dw 0BB01h
DRAM:401AB8A2                 dw 0BD01h
DRAM:401AB8A4                 dw 0BF01h
DRAM:401AB8A6                 dw 0C101h
DRAM:401AB8A8                 dw 0C301h
DRAM:401AB8AA                 dw 0C501h
DRAM:401AB8AC                 dw 0C701h
DRAM:401AB8AE                 dw 0C901h
DRAM:401AB8B0                 dw 0CB01h
DRAM:401AB8B2                 dw 0CD01h
DRAM:401AB8B4                 dw 0CF01h
DRAM:401AB8B6                 dw 0D101h
DRAM:401AB8B8                 dw 0D301h
DRAM:401AB8BA                 dw 0D501h
DRAM:401AB8BC                 dw 0D701h
DRAM:401AB8BE                 dw 0D901h
DRAM:401AB8C0                 dw 0DB01h
DRAM:401AB8C2                 dw 0DD01h
DRAM:401AB8C4                 dw 0DF01h
DRAM:401AB8C6                 dw 0E101h
DRAM:401AB8C8                 dw 0E301h
DRAM:401AB8CA                 dw 0E501h
DRAM:401AB8CC                 dw 0E701h
DRAM:401AB8CE                 dw 0E901h
DRAM:401AB8D0                 dw 0EB01h
DRAM:401AB8D2                 dw 0ED01h
DRAM:401AB8D4                 dw 0EF01h
DRAM:401AB8D6                 dw 0F101h
DRAM:401AB8D8                 dw 0F301h
DRAM:401AB8DA                 dw 0F501h
DRAM:401AB8DC                 dw 0F701h
DRAM:401AB8DE                 dw 0F901h
DRAM:401AB8E0                 dw 0FB01h
DRAM:401AB8E2                 dw 0FD01h
DRAM:401AB8E4                 dw 2FEh
DRAM:401AB8E6                 dw 4FEh
DRAM:401AB8E8                 dw 6FEh
DRAM:401AB8EA                 dw 8FEh
DRAM:401AB8EC                 dw 0AFEh
DRAM:401AB8EE                 dw 0CFEh
DRAM:401AB8F0                 dw 0EFEh
DRAM:401AB8F2                 dw 10FEh
DRAM:401AB8F4                 dw 12FEh
DRAM:401AB8F6                 dw 14FEh
DRAM:401AB8F8                 dw 16FEh
DRAM:401AB8FA                 dw 18FEh
DRAM:401AB8FC                 dw 1AFEh
DRAM:401AB8FE                 dw 1CFEh
DRAM:401AB900                 dw 1EFEh
DRAM:401AB902                 dw 20FEh
DRAM:401AB904                 dw 22FEh
DRAM:401AB906                 dw 24FEh
DRAM:401AB908                 dw 26FEh
DRAM:401AB90A                 dw 28FEh
DRAM:401AB90C                 dw 2AFEh
DRAM:401AB90E                 dw 2CFEh
DRAM:401AB910                 dw 2EFEh
DRAM:401AB912                 dw 30FEh
DRAM:401AB914                 dw 32FEh
DRAM:401AB916                 dw 34FEh
DRAM:401AB918                 dw 36FEh
DRAM:401AB91A                 dw 38FEh
DRAM:401AB91C                 dw 3AFEh
DRAM:401AB91E                 dw 3CFEh
DRAM:401AB920                 dw 3EFEh
DRAM:401AB922                 dw 40FEh
DRAM:401AB924                 dw 42FEh
DRAM:401AB926                 dw 44FEh
DRAM:401AB928                 dw 46FEh
DRAM:401AB92A                 dw 48FEh
DRAM:401AB92C                 dw 4AFEh
DRAM:401AB92E                 dw 4CFEh
DRAM:401AB930                 dw 4EFEh
DRAM:401AB932                 dw 50FEh
DRAM:401AB934                 dw 52FEh
DRAM:401AB936                 dw 54FEh
DRAM:401AB938                 dw 56FEh
DRAM:401AB93A                 dw 58FEh
DRAM:401AB93C                 dw 5AFEh
DRAM:401AB93E                 dw 5CFEh
DRAM:401AB940                 dw 5EFEh
DRAM:401AB942                 dw 60FEh
DRAM:401AB944                 dw 62FEh
DRAM:401AB946                 dw 64FEh
DRAM:401AB948                 dw 66FEh
DRAM:401AB94A                 dw 68FEh
DRAM:401AB94C                 dw 6AFEh
DRAM:401AB94E                 dw 6CFEh
DRAM:401AB950                 dw 6EFEh
DRAM:401AB952                 dw 70FEh
DRAM:401AB954                 dw 72FEh
DRAM:401AB956                 dw 74FEh
DRAM:401AB958                 dw 76FEh
DRAM:401AB95A                 dw 78FEh
DRAM:401AB95C                 dw 7AFEh
DRAM:401AB95E                 dw 7CFEh
DRAM:401AB960                 dw 7EFEh
DRAM:401AB962                 dw 80FEh
DRAM:401AB964                 dw 82FEh
DRAM:401AB966                 dw 84FEh
DRAM:401AB968                 dw 86FEh
DRAM:401AB96A                 dw 88FEh
DRAM:401AB96C                 dw 8AFEh
DRAM:401AB96E                 dw 8CFEh
DRAM:401AB970                 dw 8EFEh
DRAM:401AB972                 dw 90FEh
DRAM:401AB974                 dw 92FEh
DRAM:401AB976                 dw 94FEh
DRAM:401AB978                 dw 96FEh
DRAM:401AB97A                 dw 98FEh
DRAM:401AB97C                 dw 9AFEh
DRAM:401AB97E                 dw 9CFEh
DRAM:401AB980                 dw 9EFEh
DRAM:401AB982                 dw 0A0FEh
DRAM:401AB984                 dw 0A2FEh
DRAM:401AB986                 dw 0A4FEh
DRAM:401AB988                 dw 0A6FEh
DRAM:401AB98A                 dw 0A8FEh
DRAM:401AB98C                 dw 0AAFEh
DRAM:401AB98E                 dw 0ACFEh
DRAM:401AB990                 dw 0AEFEh
DRAM:401AB992                 dw 0B0FEh
DRAM:401AB994                 dw 0FFh
DRAM:401AB996                 dw 2FFh
DRAM:401AB998                 dw 4FFh
DRAM:401AB99A                 dw 6FFh
DRAM:401AB99C                 dw 8FFh
DRAM:401AB99E                 dw 0AFFh
DRAM:401AB9A0                 dw 0CFFh
DRAM:401AB9A2                 dw 0EFFh
DRAM:401AB9A4                 dw 10FFh
DRAM:401AB9A6                 dw 12FFh
DRAM:401AB9A8                 dw 14FFh
DRAM:401AB9AA                 dw 16FFh
DRAM:401AB9AC                 dw 18FFh
DRAM:401AB9AE                 dw 1AFFh
DRAM:401AB9B0                 dw 1CFFh
DRAM:401AB9B2                 dw 1EFFh
DRAM:401AB9B4                 dw 20FFh
DRAM:401AB9B6                 dw 22FFh
DRAM:401AB9B8                 dw 24FFh
DRAM:401AB9BA                 dw 26FFh
DRAM:401AB9BC                 dw 28FFh
DRAM:401AB9BE                 dw 2AFFh
DRAM:401AB9C0                 dw 2CFFh
DRAM:401AB9C2                 dw 2EFFh
DRAM:401AB9C4                 dw 30FFh
DRAM:401AB9C6                 dw 32FFh
DRAM:401AB9C8                 dw 34FFh
DRAM:401AB9CA                 dw 36FFh
DRAM:401AB9CC                 dw 38FFh
DRAM:401AB9CE                 dw 3AFFh
DRAM:401AB9D0                 dw 3CFFh
DRAM:401AB9D2                 dw 3EFFh
DRAM:401AB9D4                 dw 40FFh
DRAM:401AB9D6                 dw 42FFh
DRAM:401AB9D8                 dw 44FFh
DRAM:401AB9DA                 dw 46FFh
DRAM:401AB9DC                 dw 48FFh
DRAM:401AB9DE                 dw 4AFFh
DRAM:401AB9E0                 dw 4CFFh
DRAM:401AB9E2                 dw 4EFFh
DRAM:401AB9E4                 dw 50FFh
DRAM:401AB9E6                 dw 52FFh
DRAM:401AB9E8                 dw 54FFh
DRAM:401AB9EA                 dw 56FFh
DRAM:401AB9EC                 dw 58FFh
DRAM:401AB9EE                 dw 5AFFh
DRAM:401AB9F0                 dw 5CFFh
DRAM:401AB9F2                 dw 5EFFh
DRAM:401AB9F4                 dw 60FFh
DRAM:401AB9F6                 dw 62FFh
DRAM:401AB9F8                 dw 65FFh
DRAM:401AB9FA                 dw 67FFh
DRAM:401AB9FC                 dw 69FFh
DRAM:401AB9FE                 dw 6BFFh
DRAM:401ABA00                 dw 6DFFh
DRAM:401ABA02                 dw 6FFFh
DRAM:401ABA04                 dw 71FFh
DRAM:401ABA06                 dw 73FFh
DRAM:401ABA08                 dw 75FFh
DRAM:401ABA0A                 dw 77FFh
DRAM:401ABA0C                 dw 79FFh
DRAM:401ABA0E                 dw 7BFFh
DRAM:401ABA10                 dw 7DFFh
DRAM:401ABA12                 dw 7FFFh
DRAM:401ABA14                 dw 81FFh
DRAM:401ABA16                 dw 83FFh
DRAM:401ABA18                 dw 0A0FFh
DRAM:401ABA1A                 dw 0A3FFh
DRAM:401ABA1C                 dw 0D4FFh
DRAM:401ABA1E                 dw 0DCFFh
DRAM:401ABA20                 dw 0E0FFh
DRAM:401ABA22                 dw 0E2FFh
DRAM:401ABA24                 dw 0E4FFh
DRAM:401ABA26                 dw 0E6FFh
DRAM:401ABA28                 dw 0E8FFh
DRAM:401ABA2A                 dw 0EAFFh
DRAM:401ABA2C                 dw 0ECFFh
DRAM:401ABA2E                 dw 0EEFFh
DRAM:401ABA30                 dw 0F0FFh
DRAM:401ABA32                 dw 0F2FFh
DRAM:401ABA34                 dw 0F4FFh
DRAM:401ABA36                 dw 0F8FFh
DRAM:401ABA38                 dw 0FAFFh
DRAM:401ABA3A                 dw 0FCFFh
DRAM:401ABA3C                 dw 0FEFFh
DRAM:401ABA3E                 dw 0

