import { useEffect, useMemo, useState } from "react";
import LanguageContext from "./LanguageContext";

const LANGUAGE_STORAGE_KEY = "rond-security-language";

const englishTranslations = {
  "Tableau de bord": "Dashboard",
  "Gestion des agents": "Agent management",
  "Gestion des Agents": "Agent management",
  Agents: "Agents",
  Appareils: "Devices",
  "Gestion des appareils": "Device management",
  "Points de contrôle": "Checkpoints",
  "Point de contrôle": "Checkpoint",
  "Rapports d'audit": "Audit reports",
  "Rapport d'audit": "Audit report",
  Paramètres: "Settings",
  "Paramètres système": "System settings",
  "Paramètres systèmes": "System settings",
  Profil: "Profile",
  "Page introuvable": "Page not found",
  Connexion: "Sign in",
  "Erreur 404": "Error 404",
  "La page que vous recherchez n’existe pas ou a été déplacée. Vérifiez l’adresse ou retournez au tableau de bord.":
    "The page you are looking for does not exist or has been moved. Check the address or return to the dashboard.",
  "Retour au tableau de bord": "Back to dashboard",
  "Page précédente": "Previous page",
  "Mode sombre": "Dark mode",
  Activé: "On",
  Désactivé: "Off",
  Langue: "Language",
  Notifications: "Notifications",
  Activées: "On",
  Désactivées: "Off",
  Sécurité: "Security",
  "2FA activé": "2FA enabled",
  Logo: "Logo",
  "Cliquer pour modifier": "Click to change",
  "À propos": "About",
  "Cette application web est conçue pour administrer et superviser les rondes de sécurité. Elle centralise les journaux d’activité (logs), assure le suivi des scans RFID aux points de contrôle et permet de consulter l’historique des passages. Elle facilite la détection des scans manquants, le suivi des agents et la traçabilité des opérations afin d’assurer une gestion fiable et efficace des rondes de surveillance.":
    "This web application is designed to manage and supervise security patrols. It centralizes activity logs, tracks RFID scans at checkpoints, and provides access to patrol history. It helps detect missed scans, track agents, and ensure traceability for reliable and efficient patrol management.",
  "Passages confirmés": "Confirmed check-ins",
  "Évolution sur les 7 derniers jours": "Activity over the last 7 days",
  "Logs récents": "Recent logs",
  "Derniers événements enregistrés par les dispositifs":
    "Latest events recorded by devices",
  "Exporter logs": "Export logs",
  Alertes: "Alerts",
  "Derniers événements détectés sur le terrain.":
    "Latest events detected in the field.",
  "Aucune alerte enregistrée.": "No alerts recorded.",
  "Aucun message": "No message",
  "Emplacement inconnu": "Unknown location",
  "ID Agent": "Agent ID",
  "Nom agent": "Agent name",
  "Nom appareil": "Device name",
  Emplacement: "Location",
  Statut: "Status",
  Actions: "Actions",
  "Date & heure": "Date & time",
  "Date et heure": "Date and time",
  Validé: "Validated",
  "Non validé": "Not validated",
  "Aucun résultat trouvé.": "No results found.",
  "vs hier": "vs yesterday",
  Valeur: "Value",
  "Impossible de charger les statistiques du tableau de bord.":
    "Unable to load dashboard statistics.",
  "Impossible de charger les alertes.": "Unable to load alerts.",
  "Impossible de charger les passages confirmés.":
    "Unable to load confirmed check-ins.",
  "Impossible de charger les agents.": "Unable to load agents.",
  "Impossible de charger les appareils.": "Unable to load devices.",
  "Impossible de charger les points de contrôle.":
    "Unable to load checkpoints.",
  "Impossible de charger la liste des agents.":
    "Unable to load the agent list.",
  "Impossible de charger les logs.": "Unable to load logs.",
  "Impossible de charger la liste des appareils.":
    "Unable to load the device list.",
  "Résultats (": "Results (",
  "Nouvel agent": "New agent",
  "Recherche...": "Search...",
  "Chargement des agents...": "Loading agents...",
  "Modifier un agent": "Edit agent",
  "Ajout d'un nouvel agent": "Add a new agent",
  Annuler: "Cancel",
  "Enregistrement...": "Saving...",
  Enregistrer: "Save",
  Ajouter: "Add",
  "Renseignez les informations nécessaires pour créer ou modifier la fiche de l'agent.":
    "Enter the information needed to create or update the agent record.",
  "Matricule de l'agent": "Agent ID",
  "Ex. : AG-061": "E.g. AG-061",
  "Nom complet": "Full name",
  "Ex. : Rakoto Jean": "E.g. John Smith",
  Téléphone: "Phone",
  "Téléphone (facultatif)": "Phone (optional)",
  "Ex. : 0341234567": "E.g. 5551234567",
  "Confirmer la suppression": "Confirm deletion",
  "Suppression...": "Deleting...",
  Supprimer: "Delete",
  "Le matricule et le nom complet sont obligatoires.":
    "The agent ID and full name are required.",
  "Ce matricule existe déjà.": "This agent ID already exists.",
  "Agent modifié avec succès.": "Agent updated successfully.",
  "Statut de l’agent": "Agent status",
  "Activer l’agent": "Activate agent",
  "Désactiver l’agent": "Deactivate agent",
  "Impossible de modifier le statut de cet agent.":
    "Unable to update this agent's status.",
  "Agent créé avec succès.": "Agent created successfully.",
  "Agent supprimé avec succès.": "Agent deleted successfully.",
  "Impossible de créer cet agent.": "Unable to create this agent.",
  "Impossible de modifier cet agent.": "Unable to update this agent.",
  "Impossible de supprimer cet agent.": "Unable to delete this agent.",
  "Code appareil": "Device code",
  "Matricule agent": "Agent ID",
  Actif: "Active",
  Inactif: "Inactive",
  Modifier: "Edit",
  "Nouvel appareil": "New device",
  "Rechercher un appareil...": "Search devices...",
  "Chargement des appareils...": "Loading devices...",
  "Aucun appareil ne correspond à votre recherche.":
    "No devices match your search.",
  "Aucun appareil enregistré.": "No devices registered.",
  "Modifier un appareil": "Edit device",
  "Ajout d'un appareil": "Add a device",
  "Enregistrez un appareil et associez-le à un agent responsable des rondes de sécurité.":
    "Register a device and assign it to an agent responsible for security patrols.",
  "Code de l'appareil": "Device code",
  "Ex. : PN-001": "E.g. PN-001",
  "Nom de l'agent": "Agent name",
  "Rechercher par nom ou matricule...": "Search by name or agent ID...",
  "Aucun agent trouvé.": "No agents found.",
  "Le code de l'appareil et le nom de l'agent sont obligatoires.":
    "The device code and agent name are required.",
  "Ce code appareil existe déjà.": "This device code already exists.",
  "Appareil modifié avec succès.": "Device updated successfully.",
  "Appareil créé avec succès.": "Device created successfully.",
  "Appareil supprimé avec succès.": "Device deleted successfully.",
  "Impossible de créer cet appareil.": "Unable to create this device.",
  "Impossible de modifier cet appareil.": "Unable to update this device.",
  "Statut de l’appareil": "Device status",
  "Activer l’appareil": "Activate device",
  "Désactiver l’appareil": "Deactivate device",
  "Impossible de modifier le statut de cet appareil.":
    "Unable to update this device's status.",
  "Impossible de supprimer cet appareil.": "Unable to delete this device.",
  "Gestion des points de contrôle": "Checkpoint management",
  "Nouveau point de contrôle": "New checkpoint",
  "Rechercher un point de contrôle...": "Search checkpoints...",
  "Chargement des points de contrôle...": "Loading checkpoints...",
  "Aucun point de contrôle ne correspond à votre recherche.":
    "No checkpoints match your search.",
  "Aucun point de contrôle enregistré.": "No checkpoints registered.",
  "Code du point": "Checkpoint code",
  "Nom du point de contrôle": "Checkpoint name",
  "Agent responsable": "Assigned agent",
  "Non attribué": "Unassigned",
  "Modifier un point de contrôle": "Edit checkpoint",
  "Création d'un point de contrôle": "Create a checkpoint",
  Créer: "Create",
  "Enregistrez un point de contrôle et associez-le à un agent responsable des rondes de sécurité.":
    "Register a checkpoint and assign it to an agent responsible for security patrols.",
  "Code du point de contrôle": "Checkpoint code",
  "Ex. : CP-011": "E.g. CP-011",
  "Ex. : Parking": "E.g. Parking lot",
  "Nom de l'agent responsable": "Assigned agent name",
  "Rechercher par nom ou code agent...": "Search by name or agent ID...",
  "Code agent : ": "Agent ID: ",
  "Aucun agent correspondant.": "No matching agents.",
  "Le code, le nom du point et le nom de l'agent sont obligatoires.":
    "The checkpoint code, name, and assigned agent are required.",
  "Ce code de point de contrôle existe déjà.":
    "This checkpoint code already exists.",
  "Point de contrôle modifié avec succès.":
    "Checkpoint updated successfully.",
  "Point de contrôle créé avec succès.": "Checkpoint created successfully.",
  "Point de contrôle supprimé avec succès.":
    "Checkpoint deleted successfully.",
  "Impossible de modifier ce point de contrôle.":
    "Unable to update this checkpoint.",
  "Statut du point de contrôle": "Checkpoint status",
  "Activer le point de contrôle": "Activate checkpoint",
  "Désactiver le point de contrôle": "Deactivate checkpoint",
  "Impossible de modifier le statut de ce point de contrôle.":
    "Unable to update this checkpoint's status.",
  "Impossible de créer ce point de contrôle.":
    "Unable to create this checkpoint.",
  "Impossible de supprimer ce point de contrôle.":
    "Unable to delete this checkpoint.",
  "Êtes-vous sûr de vouloir supprimer le point de contrôle ":
    "Are you sure you want to delete checkpoint ",
  "Cette action est irréversible.": "This action cannot be undone.",
  "Événements enregistrés par les dispositifs":
    "Events recorded by devices",
  "Générer rapport": "Generate report",
  "Rechercher un appareil, un agent...": "Search devices, agents...",
  "Filtrer les rapports": "Filter reports",
  "Ex. Jean Dupont": "E.g. John Smith",
  Appareil: "Device",
  "Tous les appareils": "All devices",
  "Ex. Salle B": "E.g. Room B",
  "Filtrage...": "Filtering...",
  "Appliquer les filtres": "Apply filters",
  "Effacer les filtres": "Clear filters",
  "Chargement des logs...": "Loading logs...",
  "Aucun log ne correspond à vos critères.":
    "No logs match your criteria.",
  "Aucun événement enregistré.": "No events recorded.",
  "Exporter les logs": "Export logs",
  Exporter: "Export",
  "Exportation...": "Exporting...",
  "Téléchargez les journaux de ronde pour les consulter, les archiver ou les analyser.":
    "Download patrol logs to review, archive, or analyze them.",
  "Choisissez le format d'exportation :": "Choose an export format:",
  "ligne(s) seront exportées.": "row(s) will be exported.",
  "Rapport exporté avec succès.": "Report exported successfully.",
  "Une erreur est survenue lors de l'exportation.":
    "An error occurred while exporting.",
  "Date de réception": "Received at",
  "UUID événement": "Event UUID",
  "Nom": "Name",
  "Nom d'utilisateur": "Username",
  "E-mail": "Email",
  "Gérez vos informations personnelles et votre sécurité.":
    "Manage your personal information and security.",
  "Choisissez un nouveau mot de passe pour votre compte.":
    "Choose a new password for your account.",
  "Mot de passe actuel": "Current password",
  "Nouveau mot de passe": "New password",
  "8 caractères minimum": "At least 8 characters",
  "Modification...": "Updating...",
  "Changer le mot de passe": "Change password",
  "Chargement du profil...": "Loading profile...",
  "Impossible de charger le profil.": "Unable to load profile.",
  "Profil mis à jour avec succès.": "Profile updated successfully.",
  "Erreur lors de la mise à jour du profil.":
    "An error occurred while updating the profile.",
  "Veuillez renseigner les deux mots de passe.":
    "Please enter both passwords.",
  "Le nouveau mot de passe doit contenir au moins 8 caractères.":
    "The new password must be at least 8 characters long.",
  "Mot de passe modifié avec succès.": "Password changed successfully.",
  "Impossible de modifier le mot de passe.":
    "Unable to change the password.",
  "Enregistrer le profil": "Save profile",
  "Connexion réussie :": "Sign-in successful:",
  "CONNEXION": "SIGN IN",
  "Bienvenue chez": "Welcome to",
  "Connectez-vous pour continuer.": "Sign in to continue.",
  "Rondix": "Rondix",
  "La vigilance avance. Vos rondes restent sous contrôle.":
    "Stay one step ahead. Keep every patrol in control.",
  "Besoin d’aide pour vous connecter ?":
    "Need help signing in?",
  "Demande d’aide à la connexion": "Sign-in assistance request",
  "Bonjour, je rencontre un problème pour me connecter à mon compte Rondix. Pouvez-vous m’aider ?":
    "Hello, I’m having trouble signing in to my Rondix account. Can you help me?",
  "Illustration d’une carte intelligente": "Smart card illustration",
  "Connexion induisponible !": "Connection unavailable!",
  "Connectez-vous à votre compte pour accéder à votre espace de supervision.":
    "Sign in to your account to access your monitoring dashboard.",
  "Mot de passe": "Password",
  "Saisissez votre mot de passe": "Enter your password",
  "Ex. : admin": "E.g. admin",
  "Mot de passe oublié ?": "Forgot password?",
  "Veuillez renseigner tous les champs obligatoires.":
    "Please complete all required fields.",
  "Nom d'utilisateur ou mot de passe incorrect.":
    "Incorrect username or password.",
  "Impossible de se connecter au serveur. Veuillez réessayer.":
    "Unable to connect to the server. Please try again.",
  "Connexion en cours...": "Signing in...",
  "Se connecter": "Sign in",
  "MOT DE PASSE OUBLIÉ ?": "FORGOT PASSWORD?",
  "Pas d'inquiétude. Saisissez votre adresse e-mail et nous vous enverrons un code de vérification pour réinitialiser votre mot de passe.":
    "No worries. Enter your email address and we'll send you a verification code to reset your password.",
  "Adresse E-mail": "Email address",
  "Envoyer": "Send",
  "Envoi en cours...": "Sending...",
  Retour: "Back",
  "CRÉEZ UN NOUVEAU MOT DE PASSE": "CREATE A NEW PASSWORD",
  "Votre adresse e-mail a été vérifiée. Choisissez un nouveau mot de passe sécurisé pour retrouver l'accès à votre compte.":
    "Your email address has been verified. Choose a new secure password to regain access to your account.",
  "Confirmer mot de passe": "Confirm password",
  "Votre mot de passe doit contenir au moins 8 caractères. Utilisez une combinaison de lettres, de chiffres et de caractères spéciaux pour renforcer sa sécurité.":
    "Your password must be at least 8 characters long. Use a combination of letters, numbers, and special characters to make it stronger.",
  "Mise à jour...": "Updating...",
  "Réinitialiser le mot de passe": "Reset password",
  "VÉRIFIEZ VOTRE ADRESSE E-MAIL": "VERIFY YOUR EMAIL ADDRESS",
  "Nous avons envoyé un code de vérification à votre adresse e-mail. Saisissez le code reçu pour confirmer votre identité et continuer la réinitialisation de votre mot de passe.":
    "We sent a verification code to your email address. Enter the code to confirm your identity and continue resetting your password.",
  "Code de vérification": "Verification code",
  "code de vérification": "Verification code",
  "Vous n'avez pas reçu le code ?": "Didn't receive the code?",
  Renvoyer: "Resend",
  "Vérification...": "Verifying...",
  "Vérifier le code": "Verify code",
  "Un nouveau code a été envoyé.": "A new code has been sent.",
  Déconnexion: "Sign out",
  "Déconnexion réussie.": "Signed out successfully.",
  "Erreur lors de la déconnexion.": "Unable to sign out.",
  "Navigation principale": "Main navigation",
  "Fermer la fenêtre": "Close dialog",
  "Ouvrir les filtres": "Open filters",
  "Masquer le mot de passe": "Hide password",
  "Afficher le mot de passe": "Show password",
  "Saisissez votre texte": "Enter text",
  "Entrez votre adresse e-mail": "Enter your email address",
  "Rechercher...": "Search...",
  "Entrez votre mot de passe": "Enter your password",
  "Veuillez sélectionner une image valide.": "Please select a valid image.",
  "L'image ne doit pas dépasser 5 Mo.": "The image must not exceed 5 MB.",
  "Photo de profil": "Profile photo",
  "Avatar par défaut": "Default avatar",
  "Choisir une photo de profil": "Choose a profile photo",
  "Changer la photo": "Change photo",
  "Veuillez sélectionner un fichier PNG ou SVG.":
    "Please select a PNG or SVG file.",
  "Le fichier ne doit pas dépasser 5 Mo.": "The file must not exceed 5 MB.",
  "Ouvrir les paramètres de sécurité": "Open security settings",
  "Logo sélectionné": "Selected logo",
  "Logo actuel": "Current logo",
  "Revenir à la page d'accueil": "Return to home page",
  "Choisir la langue": "Choose a language",
  Fermer: "Close",
  "Sélectionnez la langue de l’interface.": "Select the interface language.",
  "Désactiver le mode sombre": "Turn off dark mode",
  "Activer le mode sombre": "Turn on dark mode",
  "Activer les notifications": "Enable notifications",
  "Fermer le menu": "Close menu",
  "Ouvrir le menu": "Open menu",
  "Ouvrir les paramètres du profil": "Open profile settings",
  "non lues": "unread",
  "Rondes confirmées aujourd'hui": "Rounds confirmed today",
  "Dispositifs actifs": "Active devices",
  "Taux de succès d'envoi": "Delivery success rate",
  "ID Agent :": "Agent ID:",
  "Nom agent :": "Agent name:",
  "Emplacement :": "Location:",
  "Heure :": "Time:",
  "Téléphone :": "Phone:",
  "Ronde de sécurité mobile": "Mobile security patrol",
  "Erreur lors de la déconnexion :": "Sign-out error:",
  "Erreur de chargement des agents :": "Error loading agents:",
  "Erreur de chargement des appareils :": "Error loading devices:",
  "Erreur de chargement des points de contrôle :":
    "Error loading checkpoints:",
  "Erreur de chargement des logs :": "Error loading logs:",
  "Erreur d'enregistrement de l'agent :": "Error saving agent:",
  "Erreur d'enregistrement de l'appareil :": "Error saving device:",
  "Erreur d'enregistrement du point de contrôle:":
    "Error saving checkpoint:",
  "Erreur d'enregistrement du point de contrôle :":
    "Error saving checkpoint:",
  "Erreur de suppression :": "Error deleting:",
  "Erreur de suppression de l'appareil :": "Error deleting device:",
  "Erreur de suppression du point de contrôle :":
    "Error deleting checkpoint:",
  "Erreur lors de l'exportation :": "Error exporting report:",
  "Le backend n'a pas renvoyé une liste d'agents.":
    "The backend did not return a valid agent list.",
  "La liste des agents reçue est invalide.":
    "The received agent list is invalid.",
  "Le serveur n'a pas renvoyé une liste valide d'agents.":
    "The server did not return a valid agent list.",
  "Le serveur n'a pas renvoyé une liste valide d'appareils.":
    "The server did not return a valid device list.",
  "Le serveur n'a pas renvoyé une liste valide de points de contrôle.":
    "The server did not return a valid checkpoint list.",
  "Le serveur n'a pas renvoyé une liste valide de logs.":
    "The server did not return a valid log list.",
  "Notification cliquée": "Notification clicked",
  "Aucune donnée à exporter.": "There is no data to export.",
  "Rapport des rondes de sécurité": "Security patrol report",
  "Historique des événements enregistrés par les appareils de ronde.":
    "History of events recorded by patrol devices.",
  Agent: "Agent",
  Date: "Date",
  "Résultats": "Results",
  "Le mot de passe doit contenir au moins 8 caractères.":
    "The password must be at least 8 characters long.",
  "Votre mot de passe a été réinitialisé avec succès !":
    "Your password has been reset successfully!",
  "Êtes-vous sûr de vouloir supprimer l'appareil":
    "Are you sure you want to delete device",
  "Êtes-vous sûr de vouloir supprimer le point de contrôle":
    "Are you sure you want to delete checkpoint",
  "Êtes-vous sûr de vouloir supprimer ": "Are you sure you want to delete ",
  "l'agent": "agent",
  "cet élément": "this item",
};

const getInitialLanguage = () => {
  const savedLanguage = window.localStorage.getItem(LANGUAGE_STORAGE_KEY);
  return savedLanguage === "en" ? "en" : "fr";
};

export function LanguageProvider({ children }) {
  const [language, setLanguage] = useState(getInitialLanguage);

  useEffect(() => {
    window.localStorage.setItem(LANGUAGE_STORAGE_KEY, language);
    document.documentElement.lang = language;
  }, [language]);

  const value = useMemo(
    () => ({
      language,
      setLanguage,
      t: (text) =>
        language === "en" ? (englishTranslations[text] ?? text) : text,
    }),
    [language],
  );

  return (
    <LanguageContext.Provider value={value}>
      {children}
    </LanguageContext.Provider>
  );
}
