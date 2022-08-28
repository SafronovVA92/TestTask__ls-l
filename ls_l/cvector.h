#ifndef CVECTOR_H
#define CVECTOR_H

// Lightweight ANSI C implementation of std::vector from C++
// Just for my convinience ;-)

#define CVECTOR_DEFAULT_capacity							(32)

typedef struct s_vector
{
	void* data;
	size_t capacity;
	size_t size;
} t_vector;


#define CVECTOR_INIT(vec, type_t)						{ vec.data = malloc(sizeof(type_t) * CVECTOR_DEFAULT_capacity); vec.capacity = CVECTOR_DEFAULT_capacity; vec.size = 0; }

#define CVECTOR_PUSHBACK(vec, type_t, value)			{ \
	if (vec.size >= vec.capacity) \
	{ \
		void* newdata = malloc(sizeof(type_t) * (vec.capacity * 2)); \
		memcpy(newdata, vec.data, vec.capacity * sizeof(type_t)); \
		free(vec.data); vec.data = newdata; vec.capacity *= 2; \
		 \
	} \
	*((type_t*)vec.data + vec.size++) = value; \
}

#define CVECTOR_GET(vec, type_t, index)					(*((type_t*)vec.data + index))

#define CVECTOR_SIZE(vec)								(vec.size)

#define CVECTOR_FREE(vec)								{ free(vec.data); vec.data = NULL; vec.capacity = vec.size = 0; }

#endif // CVECTOR_H
