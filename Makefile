############################## COMPILE VAR #####################################

CC			=	gcc
#CFLAGS		=	-Wall -Wextra -Werror
CFLAGS		=	-Wall -Wextra -Werror -g -fsanitize=address,undefined
HDIR		=	includes
SRCDIR		=	src
SUB1D		=	libft
HFLAGS		=	-I $(HDIR) -I $(SUB1D)/$(HDIR)
LIBS		=	$(SUB1D)/libft.a
NAME		=	ft_ping

############################## SOURCES #########################################

SRCC			=	options.c\
					main.c\
					ping_internals.c\
					ping.c\

ODIR			=	obj
OBJ				=	$(patsubst %.c,%.o,$(SRCC))

vpath			%.o	$(ODIR)
vpath			%.h	$(HDIR)
vpath			%.h	$(SUB1D)/$(HDIR)
vpath			%.c	$(SRCDIR)

############################## BUILD ###########################################

all: $(NAME)

$(NAME): $(SUB1D)/libft.a $(ODIR) $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(patsubst %.o,$(ODIR)/%.o,$(OBJ)) $(LIBS)

$(SUB1D)/libft.a:
	make -C $(SUB1D)

options.o: ft_ping.h libft.h
main.o: ft_ping.h libft.h
ping_internals.o: ft_ping.h libft.h
ping.o: ft_ping.h libft.h
%.o: %.c
	@mkdir -p $(ODIR)
	$(CC) -c $(CFLAGS) $< $(HFLAGS) -o $(ODIR)/$@

$(ODIR):
	mkdir -p $@

############################## CLEANUP #########################################

clean:
	rm -rf $(ODIR)
	make -C $(SUB1D) fclean

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
