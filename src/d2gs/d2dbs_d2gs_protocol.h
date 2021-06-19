#ifndef INCLUDED_D2DBS_D2GS_PROTOCOL_H
#define INCLUDED_D2DBS_D2GS_PROTOCOL_H


#include "bn_types.h"
#pragma pack(push, pack02, 1)


namespace pvpgn
{

	namespace d2gs
	{

		/* packet */
		typedef struct
		{
			bn_short	size;
			bn_short	type;
			bn_int		seqno;			/* seqno, set by the sender */
		} t_d2dbs_d2gs_header;

		typedef struct
		{
			t_d2dbs_d2gs_header	h;
		} t_d2dbs_d2gs_generic;


#define D2GS_D2DBS_SAVE_DATA_REQUEST		0x30
		typedef struct
		{
			t_d2dbs_d2gs_header	h;
			bn_short			datatype;
			bn_short			datalen;
			/* AccountName */
			/* CharName */
			/* RealmName */
			/* data, length = datalen */
		} t_d2gs_d2dbs_save_data_request;
#define D2GS_DATA_CHARSAVE		0x01	/* charsave */
#define D2GS_DATA_PORTRAIT		0x02	/* charinfo */

#define D2DBS_D2GS_SAVE_DATA_REPLY			0x30
		typedef struct
		{
			t_d2dbs_d2gs_header	h;
			bn_int				result;
			bn_short			datatype;
			/* CharName */
		} t_d2dbs_d2gs_save_data_reply;
#define D2DBS_SAVE_DATA_SUCCESS		0
#define D2DBS_SAVE_DATA_FAILED		1


#define D2GS_D2DBS_GET_DATA_REQUEST		0x31
		typedef struct
		{
			t_d2dbs_d2gs_header	h;
			bn_short			datatype;
			/* AccountName */
			/* CharName */
			/* RealmName */
		} t_d2gs_d2dbs_get_data_request;


#define D2DBS_D2GS_GET_DATA_REPLY		0x31
		typedef struct
		{
			t_d2dbs_d2gs_header	h;
			bn_int				result;
			bn_int				charcreatetime;
			bn_int				allowladder;
			bn_short			datatype;
			bn_short			datalen;
			/* CharName */
			/* data, length = datalen */
		} t_d2dbs_d2gs_get_data_reply;
#define D2DBS_GET_DATA_SUCCESS		0
#define D2DBS_GET_DATA_FAILED		1
#define D2DBS_GET_DATA_CHARLOCKED	2


#define D2GS_D2DBS_UPDATE_LADDER		0x32
		typedef struct
		{
			t_d2dbs_d2gs_header	h;
			bn_int				charlevel;
			bn_int				charexplow;
			bn_int				charexphigh;
			bn_short			charclass;
			bn_short			charstatus;
			/* CharName */
			/* RealmName */
		} t_d2gs_d2dbs_update_ladder;


#define D2GS_D2DBS_CHAR_LOCK		0x33
		typedef struct
		{
			t_d2dbs_d2gs_header	h;
			bn_int		lockstatus;
			/* AccountName */
			/* CharName */
			/* RealmName */
		} t_d2gs_d2dbs_char_lock;


#define D2DBS_D2GS_ECHOREQUEST		0x34
		typedef struct
		{
			t_d2dbs_d2gs_header	h;
		} t_d2dbs_d2gs_echoreq;


#define D2GS_D2DBS_ECHOREPLY		0x34
		typedef struct
		{
			t_d2dbs_d2gs_header	h;
		} t_d2gs_d2dbs_echoreply;

	}

}


#pragma pack(pop, pack02)


#endif /* INCLUDED_D2DBS_D2GS_PROTOCOL_H */