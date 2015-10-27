/*
 * microtcp.h
 *
 *  Created on: Oct 25, 2015
 *      Author: surligas
 */

#ifndef LIB_MICROTCP_H_
#define LIB_MICROTCP_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>

/**
 * Possible states of the microTCP socket
 *
 * NOTE: You can insert any other possible state
 * for your own convenience
 */
typedef enum {
	LISTEN,
	ESTABLISHED,
	CLOSING,
	CLOSED,
	INVALID
} mircotcp_state_t;


/**
 * This is the microTCP socket structure. It holds all the necessary
 * information of each microTCP socket.
 *
 * NOTE: Fill free to insert additional fields.
 */
typedef struct {
	int			sd;		/**< The underline UDP socket descriptor */
	mircotcp_state_t 	state;		/**< The state of the microTCP socket */
	size_t			init_win_size;	/**< The window size negotiated at the 3-way handshake */
	size_t			curr_win_size;	/**< The current window size */
	size_t			max_win_size;	/**< The maximum allowed window size */
} microtcp_sock_t;


/**
 * microTCP header structure
 */
typedef struct {
	uint32_t	seq_number;  /**< Sequence number */
	uint32_t	ack_number;  /**< ACK number */
	uint16_t	control;     /**< Control bits (e.g. SYN, ACK, FIN) */
	uint16_t	window;      /**< Window size in bytes */
	uint32_t	data_len;    /**< Data legth in bytes (EXCLUDING header) */
	uint32_t	future_use0; /**< 32-bits for future use */
	uint32_t	future_use1; /**< 32-bits for future use */
	uint32_t	future_use2; /**< 32-bits for future use */
	uint32_t	checksum;    /**< CRC-32 checksum, see crc32() in utils folder */
} microtcp_header_t;


microtcp_sock_t
microtcp_socket(int domain, int type, int protocol);

int
microtcp_bind(microtcp_sock_t socket, const struct sockaddr *address,
              socklen_t address_len);

int
microtcp_connect(microtcp_sock_t socket, const struct sockaddr *address,
                 socklen_t address_len);

int
microtcp_shutdown(microtcp_sock_t socket, int how);



#endif /* LIB_MICROTCP_H_ */
