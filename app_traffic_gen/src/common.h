#ifndef COMMON_H_
#define COMMON_H_

#ifdef __XC__
#define CHANEND_PARAM(param, name) param name
#else
#define CHANEND_PARAM(param, name) unsigned name
#endif

#endif /* COMMON_H_ */
