
#ifndef FASTCALL_TZ_H
#define FASTCALL_TZ_H


int fastcall_tz(uint32_t id, uint32_t token, ...);

#define fastcall_tz_ike(xxid, token, xx0, xx1, xx2) \
  fastcall_tz((xxid << 12) |  3, token, (xx0), (xx1), (xx2))

#define TZ_EXECUTIVE_ID 0x0003F001


#endif /*FASTCALL_TZ_H */
