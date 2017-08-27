provider glib {
	probe mem__alloc(void*, unsigned int, unsigned int, unsigned int);
	probe mem__realloc(void*, void *, unsigned int, unsigned int);
	probe mem__free(void*);
	probe slice__alloc(void*, unsigned int);
	probe slice__free(void*, unsigned int);
	probe quark__new(char *, unsigned int);
};
