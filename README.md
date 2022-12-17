Proyecto de la asignatura DSH en la que se implementa un contador de dominadas haciendo uso de la comunicación Wifi entre FireBeetle y M5Stick-C. Además,
de utilizar distintos sistemas como el NFC, módulo de sonido y altavoz.
 
FireBeetleClient -> contiene el código a cargar en un FireFeetle con las conexiones de módulo de audio, altavoz y gestor de NFC especificadas. Este gestiona las conexiones con ambos servidores del M5StrickC, cuenta las dominadas, inserta en la base de datos, gestiona las tarjeta y reproduce los sonidos.

M5StickCServer -> contiene el código a cargar en un M5StickC que hace de servidor wifi y servidor de estado del laser.
