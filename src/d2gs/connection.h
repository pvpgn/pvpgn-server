#ifndef INCLUDED_CONNECTION_H
#define INCLUDED_CONNECTION_H


namespace pvpgn
{

	namespace d2gs
	{

		/* for identifying by bnetd server, send just after connected */
		typedef struct
		{
			bn_byte		bnclass;
		} t_d2gs_connect;
#define CONNECT_CLASS_D2GS_TO_D2CS		0x64
#define CONNECT_CLASS_D2GS_TO_D2DBS		0x65

	}

}


#endif /* INCLUDED_CONNECTION_H */