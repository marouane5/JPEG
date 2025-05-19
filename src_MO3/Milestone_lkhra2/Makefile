# Nom de l'exécutable final
EXEC = jpeg2ppm

# Liste des fichiers source
SRC = entete_JPEG.c decodage.c quantifInv.c zigzaginv_idct.c conversion_rgb.c generate_ppm.c jpeg2ppm.c

# Liste des fichiers objets
OBJ = $(SRC:.c=.o)

# Options de compilation
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lm

# Règle par défaut : compilation
all: $(EXEC)

# Compilation de l'exécutable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Compilation des fichiers objets
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	rm -f $(OBJ) $(EXEC)

# Exécution avec une image passée en paramètre
run: $(EXEC)
ifeq ($(strip $(IMAGE)),)
	@echo "Erreur : Veuillez préciser l'image avec IMAGE=nom.jpg"
	@echo "Exemple : make run IMAGE=test.jpg"
else
	./$(EXEC) $(IMAGE)
endif

.PHONY: all clean run
