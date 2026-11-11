import NofifIcon from "../../assets/icons/alert-outlined.svg?react";
import avatar from "../../assets/icons/profile.svg";
import { useLocation } from "react-router";
import useLanguage from "../../context/useLanguage";

const pageTitles = {
  "/dashboard": "Tableau de bord",
  "/agents": "Gestion des agents",
  "/devices": "Gestion des appareils",
  "/locations": "Points de contrôle",
  "/settings": "Paramètres",
  "/reports": "Rapports d'audit",
  "/signin": "Connexion",
};

const NavBar = ({
  username = "Jhon Doe",
  photoProfile = avatar,
  notificationCount = 0,
  onNotifClick,
  onProfileClick,
  onMenuClick,
  isMenuOpen = false,
  isNotificationPanelOpen = false,
}) => {
  const location = useLocation();
  const { t } = useLanguage();

  const title = t(pageTitles[location.pathname] ?? "Page introuvable");
  return (
    <header className="flex w-full items-center justify-between gap-2 border-b bg-surface px-3 py-2 sm:px-4 sm:py-3 border-text-muted">
      <div className="flex min-w-0 items-center gap-2 sm:gap-3">
        <button
          type="button"
          aria-label={
            isMenuOpen ? t("Fermer le menu") : t("Ouvrir le menu")
          }
          aria-expanded={isMenuOpen}
          onClick={onMenuClick}
          className="inline-flex h-10 w-10 shrink-0 items-center justify-center rounded-lg text-text-secondary hover:bg-gray-100 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary md:hidden"
        >
          <svg
            aria-hidden="true"
            viewBox="0 0 24 24"
            fill="none"
            stroke="currentColor"
            strokeWidth="2"
            strokeLinecap="round"
            className="h-6 w-6"
          >
            {isMenuOpen ? (
              <path d="M18 6 6 18M6 6l12 12" />
            ) : (
              <path d="M4 6h16M4 12h16M4 18h16" />
            )}
          </svg>
        </button>
        <h1 className="truncate font-sans text-lg text-primary sm:text-2xl">
          {title}
        </h1>
      </div>
      <div className="flex shrink-0 flex-row items-center justify-end gap-2 sm:gap-4">
        <button
          type="button"
          aria-label={`${t("Notifications")} : ${notificationCount} ${t("non lues")}`}
          aria-expanded={isNotificationPanelOpen}
          onClick={onNotifClick}
          className="relative inline-flex items-center justify-center w-10 h-10 rounded-full hover:bg-gray-100"
        >
          <NofifIcon className="text-gray-600 w-7 h-7" />

          {notificationCount > 0 && (
            <span className="absolute -right-0.5 -top-0.5 flex h-5 min-w-5 items-center justify-center rounded-full bg-red-500 px-1 text-xs font-bold text-white">
              {notificationCount > 9 ? "9+" : notificationCount}
            </span>
          )}
        </button>
        <button
          type="button"
          aria-label={t("Ouvrir les paramètres du profil")}
          onClick={onProfileClick}
          className="flex items-center justify-end gap-2 rounded-lg p-1 hover:bg-gray-100 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary"
        >
          <span className="hidden font-sans text-sm text-text-secondary sm:block">
            {username}
          </span>
          <img
            src={photoProfile}
            className="h-8 w-8 rounded-full bg-text-muted"
            alt=""
          />
        </button>
      </div>
    </header>
  );
};

export default NavBar;
