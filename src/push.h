#ifndef PUSH_H
#define PUSH_H

/* Initializes persistent VAPID keys and subscription storage. */
int push_init(void);
/* Returns the URL-safe base64 VAPID public key for browser PushManager. */
int push_public_key(char *out, int size);
/* Stores/removes browser PushSubscription endpoints. */
int push_subscribe(const char *endpoint);
int push_unsubscribe(const char *endpoint);
/* Sends an empty Web Push message; the service worker fetches event data. */
int push_broadcast(const char *title, const char *body, const char *level);

#endif
