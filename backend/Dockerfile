
# Étape 1 : compilation Maven
FROM maven:3.9-eclipse-temurin-21 AS build

WORKDIR /app

# Copier les fichiers de configuration Maven
COPY pom.xml .

# Télécharger les dépendances pour faciliter le cache Docker
RUN mvn -B dependency:go-offline

# Copier le code source
COPY src ./src

# Compiler et générer le JAR
RUN mvn -B clean package -DskipTests


# Étape 2 : exécution Spring Boot
FROM eclipse-temurin:21-jre

WORKDIR /app

# Copier le JAR compilé
COPY --from=build /app/target/*.jar app.jar

# Port utilisé par Render
EXPOSE 10000

# Démarrer l'application
CMD ["java", "-XX:MaxRAMPercentage=75.0", "-jar", "app.jar"]