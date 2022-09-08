/*
 * ras.h header file for RAS functions
 */

#ifndef RAS_H_
#define RAS_H_

// Initialize the port connection to TiM1637 and the ras
void rasInit();

void rasRead(uint32_t dataSeq[]);

#endif /* RAS_H_ */
