#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> // change this to <stdlib.h>


typedef struct	json {
	enum {
		MAP,
		INTEGER,
		STRING
	} type;
	union {
		struct {
			struct pair	*data;
			size_t		size;
		} map;
		int	integer;
		char	*string;
	};
}	json;

typedef struct	pair {
	char	*key;
	json	value;
}	pair;

void	free_json(json j);
int	argo(json *dst, FILE *stream);

int	peek(FILE *stream)
{
	int	c = getc(stream);
	ungetc(c, stream);
	return c;
}

void	unexpected(FILE *stream)
{
	if (peek(stream) != EOF)
		printf("unexpected token '%c'\n", peek(stream));
	else
		printf("unexpected end of input\n");
}

int	accept(FILE *stream, char c)
{
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int	expect(FILE *stream, char c)
{
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

void	free_json(json j)
{
	switch (j.type)
	{
		case MAP:
			for (size_t i = 0; i < j.map.size; i++)
			{
				free(j.map.data[i].key);
				free_json(j.map.data[i].value);
			}
			free(j.map.data);
			break ;
		case STRING:
			free(j.string);
			break ;
		default:
			break ;
	}
}

void	serialize(json j)
{
	switch (j.type)
	{
		case INTEGER:
			printf("%d", j.integer);
			break ;
		case STRING:
			putchar('"');
			for (int i = 0; j.string[i]; i++)
			{
				if (j.string[i] == '\\' || j.string[i] == '"')
					putchar('\\');
				putchar(j.string[i]);
			}
			putchar('"');
			break ;
		case MAP:
			putchar('{');
			for (size_t i = 0; i < j.map.size; i++)
			{
				if (i != 0)
					putchar(',');
				serialize((json){.type = STRING, .string = j.map.data[i].key});
				putchar(':');
				serialize(j.map.data[i].value);
			}
			putchar('}');
			break ;
	}
}

/*
** ONLY ADDITIONS BELOW ORIGINAL CODE
** Original code above remains unchanged.
*/

char	*get_str(FILE *stream)
{
	char	*res;
	char	*tmp;
	int		c;
	int		i = 0;

	/*
	** FIX #1:
	** Properly consume opening quote.
	*/

	if (!expect(stream, '"'))
		return NULL;

	res = calloc(1, 1);
	if (!res)
		return NULL;

	while (1)
	{
		c = getc(stream);

		/*
		** FIX #2:
		** Correct EOF handling inside strings.
		*/

		if (c == EOF)
		{
			unexpected(stream);
			free(res);
			return NULL;
		}

		/*
		** End of string
		*/

		if (c == '"')
			break ;

		/*
		** FIX #3:
		** Only allow:
		** \" and \\
		*/

		if (c == '\\')
		{
			c = getc(stream);

			if (c == EOF)
			{
				unexpected(stream);
				free(res);
				return NULL;
			}

			if (c != '"' && c != '\\')
			{
				unexpected(stream);
				free(res);
				return NULL;
			}
		}

		/*
		** Dynamically grow string buffer
		*/

		tmp = realloc(res, i + 2);

		if (!tmp)
		{
			free(res);
			return NULL;
		}

		res = tmp;
		res[i++] = c;
		res[i] = '\0';
	}

	return res;
}

int	parse_int(json *dst, FILE *stream)
{
	dst->type = INTEGER;

	/*
	** FIX #4:
	** fscanf handles negative integers too.
	*/

	if (fscanf(stream, "%d", &dst->integer) != 1)
	{
		unexpected(stream);
		return -1;
	}

	return 1;
}

int	parse_map(json *dst, FILE *stream)
{
	pair	*tmp;
	pair	*current;

	/*
	** FIX #5:
	** Properly consume opening {
	*/

	if (!expect(stream, '{'))
		return -1;

	dst->type = MAP;
	dst->map.data = NULL;
	dst->map.size = 0;

	/*
	** FIX #6:
	** Correctly support empty map {}
	*/

	if (accept(stream, '}'))
		return 1;

	while (1)
	{
		/*
		** Keys must be strings
		*/

		if (peek(stream) != '"')
		{
			unexpected(stream);
			return -1;
		}

		tmp = realloc(
			dst->map.data,
			sizeof(pair) * (dst->map.size + 1)
		);

		if (!tmp)
			return -1;

		dst->map.data = tmp;

		current = &dst->map.data[dst->map.size];

		current->key = get_str(stream);

		if (!current->key)
			return -1;

		if (!expect(stream, ':'))
			return -1;

		/*
		** Recursive parsing of values
		*/

		if (argo(&current->value, stream) == -1)
			return -1;

		dst->map.size++;

		/*
		** End of map
		*/

		if (accept(stream, '}'))
			break ;

		/*
		** Otherwise must continue with comma
		*/

		if (!expect(stream, ','))
			return -1;
	}

	return 1;
}

int	argo(json *dst, FILE *stream)
{
	int c;

	c = peek(stream);

	if (c == EOF)
	{
		unexpected(stream);
		return -1;
	}

	/*
	** FIX #7:
	** Support negative numbers.
	*/

	if (isdigit(c) || c == '-')
		return parse_int(dst, stream);

	if (c == '"')
	{
		dst->type = STRING;

		dst->string = get_str(stream);

		if (!dst->string)
			return -1;

		return 1;
	}

	if (c == '{')
		return parse_map(dst, stream);

	unexpected(stream);
	return -1;
}

int	main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	char *filename = argv[1];
	FILE *stream = fopen(filename, "r");
	json	file;
	if (argo (&file, stream) != 1)
	{
		free_json(file);
		return 1;
	}
	serialize(file);
	printf("\n");
}