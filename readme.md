J'avais besoin d'un serveur http en C pour un projet perso de microservices et donc j'ai décidé de l'implémenter plutôt qu'utiliser une lib, à terme je vais faire une structure de données (un arbre en gros) pour gérer les routes.
Aussi, le but est de pouvoir créer plusieurs contrôleurs et de pouvoir avoir une base pour tout faire en http.
Un moment je me suis dit bah pourquoi pas faire en HTTP 3 mais ya pas assez de ressources sur le protocole QUIC donc la flemme.

ha et aussi je voulais le paralléliser sinon ça sert à rien, après c'est facile de paralléliser un serveur http (c'est plus juste avoir plusieurs thread qui font la même chose).
