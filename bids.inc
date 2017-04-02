#include <a_samp>
#include <FN>
#include <foreach>
#include <zcmd>
#include <sscanf2>

#pragma tabsize 0

#define public::%0(%1) 	forward%0(%1); public%0(%1)
#define SCM				SendClientMessage
#define forEx(%0,%1) 	format(%0, sizeof(%0), %1)
 
new 
	lastUser[MAX_PLAYER_NAME],
    lastPrice[MAX_PLAYERS] = -1,
    isActiveBid[MAX_PLAYERS] = -1,
	isPlayerOnBids[MAX_PLAYERS] = 0,
    bidsID = -1,
    bidsTimer = -1,
    bidsCountTimer = 120,
    bidsName[64],
    bidsPrice = -1;
    
public OnPlayerConnect(playerid)
{
	forEx(lastUser, "No One");
    lastPrice[playerid] = -1;
    isActiveBid[playerid] = -1;
	isPlayerOnBids[playerid] = 0;
	return 1;
}

public:: Bids(bidName[], const bidPrice) {
    new szString[255], bzname = strlen(bidName);
	new days = GetDayName();
	
	if(days == 6 || days == 7 || days == 1) { // Vineri | Sambata
		if(bzname < 1) forEx(bidsName, "Unknow");
        if(bzname > 1) forEx(bidsName, bidName);
		bidsPrice = bidPrice;
		bidsID ++;
		format(szString, sizeof(szString), "BIDS: Bussines {90C3D4}%s {FFFFFF}a fost scos la vanzare la pretul de {90C3D4}$%s.", bidsName, FormatNumber(bidsPrice));
        foreach(new i : Player) {
            SendClientMessage(i, -1, "------------------------------------------------");
            SendClientMessage(i, -1, szString);
            SendClientMessage(i, -1, "* Pentru a licita foloseste /bid <price>");
            SendClientMessage(i, -1, "* Licitatiile se incheie automat dupa 2 minute !");
            SendClientMessage(i, -1, "------------------------------------------------");
            bidsTimer = SetTimerEx("EndActiveBids", 1000, true, "dd", bidsID, i);
        }
	}
	
	return 1;
}

public:: EndActiveBids(bidID) {
	new szString[128];
	if(bidsCountTimer > 0) bidsCountTimer --;

	if(bidsCountTimer == 60) {
        foreach(Player, i) {
            SendClientMessage(i, -1, "------------------------------------------------");
            format(szString, sizeof(szString), "Licitatia cu ID(#%d) este activa, puteti licita in continuare !", bidsID);
            SendClientMessage(i, -1, szString);
            format(szString, sizeof(szString), "Ultimul player care a licitat este %s cu suma de $%s.", lastUser, FormatNumber(lastPrice[i]));
            SendClientMessage(i, -1, szString);
            SendClientMessage(i, -1, "------------------------------------------------");
        }
	}
	
	if(bidsCountTimer == 0) {
		if(strcmp(lastUser, "No One", true) == 0) {
            foreach(Player, i) {
                SendClientMessage(i, -1, "------------------------------------------------");
                format(szString, sizeof(szString), "Licitatia cu ID #%d a fost incheiata !", bidsID);
                SendClientMessage(i, -1, szString);
                format(szString, sizeof(szString), "Nici un player nu a licitat pentru acest bussines !");
                SendClientMessage(i, -1, szString);
                SendClientMessage(i, -1, "------------------------------------------------");
				isPlayerOnBids[i] = 0;
				lastPrice[i] = -1;
            }
		} else {
            foreach(Player, i) {
                SendClientMessage(i, -1, "------------------------------------------------");
                format(szString, sizeof(szString), "Licitatia cu ID #%d a fost incheiata !", bidsID);
                SendClientMessage(i, -1, szString);
                format(szString, sizeof(szString), "Player-ul %s a catigat aceasta licitatie. Suma licitata $%s !", lastUser, FormatNumber(lastPrice[i]));
                SendClientMessage(i, -1, szString);
                SendClientMessage(i, -1, "------------------------------------------------");
				isPlayerOnBids[i] = 0;
				lastPrice[i] = -1;
            }
		}
		
		forEx(lastUser, "No One");
		forEx(bidsName, "No One");
		
        bidsID = -1;
        bidsCountTimer = -1;
        bidsPrice = -1;
        KillTimer(bidsTimer);
        bidsTimer = -1;
	}
	return 1;
}

/*-----------------------------------------------------------------------------------------------------------*/
// Stocks
/*-----------------------------------------------------------------------------------------------------------*/
stock GetDayName(d=0,m=0,y=0) {
    if(d==0&&m==0&&y==0) {
        getdate(y, m, d);
    }

    new month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        i;

    if (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0))
        month[1] = 29;

    if (y < 1900 || m < 1 || m > 12 || d < 1 || d > month[m - 1])
        return 0;

    for (i = 1900; i < y; i++) {
        if (i % 4 == 0 && (i % 100 != 0 || i % 400 == 0))
            d += 366;
        else
            d += 365;
    }

    for (i = 0; i < m - 1; i++) {
        d += month[i];
    }

    return d%7+1;
}

/*-----------------------------------------------------------------------------------------------------------*/
// Commands
/*-----------------------------------------------------------------------------------------------------------*/
CMD:startbids(playerid, params[]) {
    new bidName[64], bidPrice;
	new days = GetDayName();

	if(days == 7 || days == 1 || days == 6) {
		if(sscanf(params, "s[64]d", bidName, bidPrice)) return SendClientMessage(playerid, -1, "Syntax: /startbids <Biz Name> <Biz Price>");
		CallRemoteFunction("Bids", "si", bidName, bidPrice);
	} else return SendClientMessage(playerid, -1, "Error: Licitatiile pot fii pornite doar in zilele [Vineri, Sambata, Duminica] !");
    return 1;
}
 
CMD:bid(playerid, params[]) {
    new suma, nume[MAX_PLAYER_NAME];
    GetPlayerName(playerid, nume, MAX_PLAYER_NAME);
    if(sscanf(params, "i", suma)) return SCM(playerid, -1, "Usage: /bid <Price>");
    if(bidsID < 0) return SCM(playerid, -1, "ERROR: Nu este o licitatie activa ");
    if(GetPlayerMoney(playerid) < bidsPrice) return SCM(playerid, -1, "ERROR: Nu detii destui bani pentru a licita.");
    if(suma > GetPlayerMoney(playerid)) return SCM(playerid, -1, "ERROR: Nu detii destui bani pentru a licita.");
    if(suma < bidsPrice) return SCM(playerid, -1, "ERROR: Nu detii destui bani pentru a licita.");
    if(suma <= lastPrice[playerid]) return SCM(playerid, -1, "ERROR: Nu detii destui bani pentru a licita.");
   
    isActiveBid[playerid] = 1;
	if(isPlayerOnBids[playerid] == 0) return isPlayerOnBids[playerid] = 1;
	
    lastPrice[playerid] = suma;
	forEx(lastUser, nume);
    GivePlayerMoney(playerid, -suma);
 
    new szString[128];
    if(isActiveBid[playerid] == 1 && isPlayerOnBids[playerid] == 1)
    {
        foreach(Player, i) {
            format(szString, sizeof(szString), "{6649CC}%s a licitat pentru bussines %s suma de $%s", lastUser[playerid], bidsName, FormatNumber(lastPrice[playerid]));
            SendClientMessage(i, -1, szString);
        }
    }
    return 1;
}
 
CMD:togbid(playerid, params[]) {
    if(isActiveBid[playerid] <= 0) {
        isActiveBid[playerid] = 1;
        SCM(playerid, -1, "{6649CC} Acum esti spectator pe o licitatie activa !");
    } else {
        isActiveBid[playerid] = 0;
        SCM(playerid, -1, "{6649CC} Acum nu mai esti spectator pe o licitatie !");
    }
    return 1;
}
