### Instructions, en ordre




python server.py

apt install mosquitto

Pour lancer le MQTT:

mosquitto -c mosquitto.conf

Si la commande au dessus ne marche pas, il faut changer la valuer 'port' dans le fichier subscriber.py et mosquitto.conf . Vous pouvez utiliser 1884 plutot que 1883 



Connecter l'ordinateur et arduino à la meme reseau. Trouver l'addresse ip de ta machine avec ifconfig. L'ip normalement commence par 192.quelquechose 

OBS: Changer l'address MQTT -> Utilisez l'addresse IP decouvert par ifconfig


python subscriber.py




lancer le arduino . il fault aussi changer l'adresse MQTT 

Il peut prendre un peu de temps s'il y a des erreurs de CORS sur l'onglet console dans le navigateur

Il peut aussi etre necessaire d'installer des libraries python




