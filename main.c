#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <MagickWand/MagickWand.h>

int main(int argc, char **argv){

  MagickWandGenesis();

  MagickWand *magick_wand = NewMagickWand();
  MagickWand *logo_wand = NewMagickWand();
  int height, width;
  long int max_height, max_width;
  float ratio;
  int opt = 0; //Compteur pour les options
  char *args; //Pointeur pour récupérer les paramètres multiples
  char *ptr; //Ptr est nécessaire pour la conversion string to long int
  char *dest = argv[2]; 
  int force = 0;

 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define ThrowWandException(wand)					\
  {									\
    char								\
      *description;							\
									\
    ExceptionType							\
      severity;								\
									\
    description = MagickGetException(wand,&severity);			\
    fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description);	\
    description = (char *) MagickRelinquishMemory(description);		\
    DestroyMagickWand(magick_wand);					\
    DestroyMagickWand(logo_wand);					\
    exit(-1);								\
  }


  if(argc < 2){
    fprintf(stderr, "Usage: %s source destination [-option] \"arg ... arg\"\n", argv[0]);
    DestroyMagickWand(magick_wand);
    DestroyMagickWand(logo_wand);
    exit(1);
  }
  

  int i = 0;
  for(i; i < argc; i++){
    if(strcmp(argv[i], "-f") == 0){
      force = 1;
      break;
    }
  }

  if(force == 0){
    if(access(dest, F_OK) != -1){
      fprintf(stderr, "Fichier déjà présent, utiliser -f pour écraser\n");
      DestroyMagickWand(magick_wand);
      DestroyMagickWand(logo_wand);
      exit(2);
    }
  }

  /*
    Lecture de l'image.
  */
  if(access(argv[1], F_OK) == 0){    
    if(!MagickReadImage(magick_wand, argv[1])){
      ThrowWandException(magick_wand);
    }
  }
  else{
    fprintf(stderr, "Fichier source (%s) introuvable\n", argv[1]);
    DestroyMagickWand(magick_wand);
    DestroyMagickWand(logo_wand);
    exit(3);
  }

  
  //Recupération de sa taille
  height = MagickGetImageHeight(magick_wand);
  width = MagickGetImageWidth(magick_wand);
  ratio =(float) width /(float) height;

  

  //Boucle d'options
  /*
    c : resize propre et crop
    r : resize à la sauvage
    a : ajout logo avec coordonnées
    f : forcer l'écrasement si l'image est déjà présente
    p : ajout logo et placement prédéfini
   */
  while((opt = getopt(argc, argv, "c:r:a:fp:")) != -1){

    switch(opt){
    
    case 'c':
      
      //Recupération de la largeur et hauteur en découpant optarg en tokens
      args = NULL;
      if(!(args = strtok(optarg, " "))){
	fprintf(stderr,"Erreur strtok argument width de c\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);
	exit(10);
      }
      if(!(max_width = strtol(args, &ptr, 10))){
	fprintf(stderr,"Erreur strtol argument width de c\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);
	exit(11);
      }

      if(!(args = strtok(NULL, " "))){
	fprintf(stderr,"Erreur strtok argument height de c\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);
	exit(12);
      }
      if(!(max_height = strtol(args, &ptr, 10))){
	fprintf(stderr,"Erreur strtol argument height de c\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(13);
      }
      
      /*
	Redimensionnement
      */        
      float zoom1 = ((float)max_width/(float)width);
      float zoom2 = ((float)max_height/(float)height);

      if(zoom1 < zoom2){
	if((zoom1*width >= max_width) && (zoom1*height >= max_height)){
	  width = zoom1*width;
	  height = zoom1*height;
	}
	else{
	  width = zoom2*width;
	  height = zoom2*height;
	}
      }
      else{
	if((zoom2*width >= max_width) && (zoom2*height >= max_height)){
	  width = zoom2*width;
	  height = zoom2*height;
	}
	else{
	  width = zoom1*width;
	  height = zoom1*height;
	}
      }

      printf("Après première étape : %d * %d\n", width, height);         
      if(!MagickResizeImage(magick_wand, width, height, LanczosFilter)){
	ThrowWandException(magick_wand);
      }
      if(!MagickSetImageCompressionQuality(magick_wand, 95)){
	ThrowWandException(magick_wand);
      }   
      
      //Crop de l'image si elle reste trop grande.
      if(height > max_height){
	if(!MagickCropImage(magick_wand, max_width, max_height, 0, (height-max_height)/2)){
	  ThrowWandException(magick_wand);
	}
      }
      else if(width > max_width){
	if(!MagickCropImage(magick_wand, max_width, max_height, (width-max_width)/2, 0)){
	  ThrowWandException(magick_wand);
	}
      }

      //Maj des dimensions
      height = MagickGetImageHeight(magick_wand);
      width = MagickGetImageWidth(magick_wand);
            
      if(!MagickSetImagePage(magick_wand, width, height, 0, 0)){
	ThrowWandException(magick_wand);
      }

      break;
	
    case 'r' :

      //Recupération de la largeur et hauteur en découpant optarg en tokens
      args = NULL;
      if(!(args = strtok(optarg, " "))){
	fprintf(stderr,"Erreur strtok argument width de r\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(20);
      }
      if(!(max_width = strtol(args, &ptr, 10))){
	fprintf(stderr,"Erreur strtol argument width de r\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);
	exit(21);
      }

      if(!(args = strtok(NULL, " "))){
	fprintf(stderr,"Erreur strtok argument height de r\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(22);
      }
      if(!(max_height = strtol(args, &ptr, 10))){
	fprintf(stderr,"Erreur strtol argument height de r\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(23);
      }

      if(!MagickResizeImage(magick_wand, max_width, max_height, LanczosFilter)){
	ThrowWandException(magick_wand);
      }

      //Maj des dimensions
      height = MagickGetImageHeight(magick_wand);
      width = MagickGetImageWidth(magick_wand);

      break;

      //Ajouter un logo sur l'image
    case 'a':
      MagickSetLastIterator(magick_wand);
     
      //Recupération de la largeur et hauteur en découpant optarg en tokens
      args = NULL;
      int x, y;
      if(!(args = strtok(optarg, " "))){
	fprintf(stderr,"Erreur strtok argument image de a\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(30);
      }
      
      if(access(args, F_OK) != -1){
	if(!MagickReadImage(logo_wand, args)){
	  ThrowWandException(logo_wand);
	}
      }
      else{
	fprintf(stderr,"Erreur, logo (%s) à incruster introuvable\n", dest);
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(31);	
      }

      if(!(args = strtok(NULL, " "))){
	fprintf(stderr,"Erreur strtok argument x de a\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(32);
      }
      if(!(x = strtol(args, &ptr, 10))){
	fprintf(stderr,"Erreur strtol argument x de a\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(33);
      }

      if(!(args = strtok(NULL, " "))){
	fprintf(stderr,"Erreur strtok argument y de a\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(34);
      }
      if(!(y = strtol(args, &ptr, 10))){
	fprintf(stderr,"Erreur strtol argument y de a\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(35);
      }
      
      //Recupération taille du logo
      int lwidth = MagickGetImageWidth(logo_wand);
      int lheight = MagickGetImageHeight(logo_wand);

      //Ajout du logo sur le fond
      if(!MagickCompositeLayers(magick_wand, logo_wand, 
				OverCompositeOp, x-lwidth/2, y-lheight/2)){
	ThrowWandException(magick_wand);
      }
      
      magick_wand = MagickCoalesceImages(magick_wand);
      if(!magick_wand)
	ThrowWandException(magick_wand);

      MagickSetLastIterator(magick_wand);

      break;

    case 'p':
      MagickSetLastIterator(magick_wand);
     
      //Recupération de la largeur et hauteur en découpant optarg en tokens
      args = NULL;
      int n;
      if(!(args = strtok(optarg, " "))){
	fprintf(stderr,"Erreur strtok argument image de p\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(40);
      }
      
      if(access(args, F_OK) != -1){
	if(!MagickReadImage(logo_wand, args)){
	  ThrowWandException(logo_wand);
	}
      }
      else{
	fprintf(stderr,"Erreur, logo (%s) à incruster introuvable p\n", args);
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(41);	
      }

      if(!(args = strtok(NULL, " "))){
	fprintf(stderr,"Erreur strtok chiffre position p\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(42);
      }
      if(!(n = strtol(args, &ptr, 10))){
	fprintf(stderr,"Erreur strtol chiffre position p\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(43);
      }

      switch(n){
      case 1:
	if(!MagickCompositeImageGravity(magick_wand, logo_wand, OverCompositeOp,
					NorthWestGravity)){
	  ThrowWandException(magick_wand);
	}

	magick_wand = MagickCoalesceImages(magick_wand);
	if(!magick_wand)
	  ThrowWandException(magick_wand);
	
	MagickSetLastIterator(magick_wand);
	
	break;
      
      case 2:
	if(!MagickCompositeImageGravity(magick_wand, logo_wand, OverCompositeOp,
					NorthGravity)){
	  ThrowWandException(magick_wand);
	}
	
	magick_wand = MagickCoalesceImages(magick_wand);
	if(!magick_wand)
	  ThrowWandException(magick_wand);
	
	MagickSetLastIterator(magick_wand);

	break;
      
      case 3:
	if(!MagickCompositeImageGravity(magick_wand, logo_wand, OverCompositeOp,
					NorthEastGravity)){
	  ThrowWandException(magick_wand);
	}

	magick_wand = MagickCoalesceImages(magick_wand);
	if(!magick_wand)
	  ThrowWandException(magick_wand);
	
	MagickSetLastIterator(magick_wand);

	break;

      case 4:
	if(!MagickCompositeImageGravity(magick_wand, logo_wand, OverCompositeOp,
					WestGravity)){
	  ThrowWandException(magick_wand);
	}

	magick_wand = MagickCoalesceImages(magick_wand);
	if(!magick_wand)
	  ThrowWandException(magick_wand);
	
	MagickSetLastIterator(magick_wand);

	break;

      case 5:
	if(!MagickCompositeImageGravity(magick_wand, logo_wand, OverCompositeOp,
					CenterGravity)){
	  ThrowWandException(magick_wand);
	}

	magick_wand = MagickCoalesceImages(magick_wand);
	if(!magick_wand)
	  ThrowWandException(magick_wand);
	
	MagickSetLastIterator(magick_wand);

	break;

      case 6:
	if(!MagickCompositeImageGravity(magick_wand, logo_wand, OverCompositeOp,
					EastGravity)){
	  ThrowWandException(magick_wand);
	}

	magick_wand = MagickCoalesceImages(magick_wand);
	if(!magick_wand)
	  ThrowWandException(magick_wand);
	
	MagickSetLastIterator(magick_wand);

	break;

      case 7:
	if(!MagickCompositeImageGravity(magick_wand, logo_wand, OverCompositeOp,
					SouthWestGravity)){  
	  ThrowWandException(magick_wand);
	}
	
	magick_wand = MagickCoalesceImages(magick_wand);
	if(!magick_wand)
	  ThrowWandException(magick_wand);
	
	MagickSetLastIterator(magick_wand);

	break;

      case 8:
	if(!MagickCompositeImageGravity(magick_wand, logo_wand, OverCompositeOp,
				    SouthGravity)){
	ThrowWandException(magick_wand);
	}

	magick_wand = MagickCoalesceImages(magick_wand);
	if(!magick_wand)
	  ThrowWandException(magick_wand);
	
	MagickSetLastIterator(magick_wand);

	break;

      case 9:
	if(!MagickCompositeImageGravity(magick_wand, logo_wand, OverCompositeOp,
					SouthEastGravity)){
	    ThrowWandException(magick_wand);
	}
	
	magick_wand = MagickCoalesceImages(magick_wand);
	if(!magick_wand)
	  ThrowWandException(magick_wand);
	
	MagickSetLastIterator(magick_wand);

	break;
	
      default:
	fprintf(stderr, "Chiffre donné doit etre compris entre 1 et 9\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(44);	
	break;
      }
      

      break;

    case '?':
      //Gestion des erreur dues à un manque de paramètres
      if (optopt == 'a') {
	fprintf(stderr, "Indiquez l'emplacement du logo en argument et les coordonnées de sa position\n_a \"chemin_logo x y\"\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(4);
      }
      else if(optopt == 'c'){
	fprintf(stderr, "Indiquez les dimensions\n-c \"width height\"\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(5);
      }
      else if(optopt == 'r'){
	fprintf(stderr, "Indiquez les dimensions\n-r \"width height\"");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(6);
      }
      else if(optopt == 'p'){
	fprintf(stderr, "Indiquez l'emplacement du logo en argument et l'entier de sa position parmis les neufs prédéfinies\n-p \"chemin_logo [1-9]\"\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(7);
      }

      break;

    }
  }

  //Sortie de la nouvelle image

  //Récupération nom du fichier
  char cmd[1024] = "mkdir ";
  char temp[1024];
  char chemin[1024];
  if(!strcpy(temp, "")){
    fprintf(stderr,"Erreur strcpy initialisation de temp\n");
    DestroyMagickWand(magick_wand);
    DestroyMagickWand(logo_wand);    
    exit(50);            
  }
  if(!strcpy(chemin, "")){
    fprintf(stderr,"Erreur strcpy initialisation de chemin\n");
    DestroyMagickWand(magick_wand);
    DestroyMagickWand(logo_wand);    
    exit(51);            
  }

  args = NULL;
  args = strtok(dest, "/");


  /*
    On parcourt args en le découpant en tokens pour récupérer l'arborescence
    et le nom du fichier séparément
  */
  while(args != NULL){

    if(!strcpy(temp, args)){
      fprintf(stderr,"Erreur strcpy récupération destination\n");
      DestroyMagickWand(magick_wand);
      DestroyMagickWand(logo_wand);
      exit(52);            
    }

    args = strtok(NULL, "/");

    if(args != NULL){

      if(!strcat(chemin, temp)){
	fprintf(stderr,"Erreur strcat récupération destination\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(53);            
      }

      if(!strcat(chemin, "/")){  
	fprintf(stderr,"Erreur strcat récupération destination\n");
	DestroyMagickWand(magick_wand);
	DestroyMagickWand(logo_wand);	
	exit(54);
      }

      system(strcat(cmd, strcat(temp, "/")));
    }

  }

  if(!MagickWriteImage(magick_wand, strcat(chemin, temp))){
    ThrowWandException(magick_wand);
  }
  DestroyMagickWand(magick_wand);
  DestroyMagickWand(logo_wand);
  MagickWandTerminus();

  return(0);    
}
