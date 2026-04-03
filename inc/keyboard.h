#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#ifdef __cplusplus
 extern "C" {
#endif

void scan_matrix(void);
void debounce(void);

#ifdef __cplusplus
 }
#endif

#endif /* KEYBOARD_H_*/