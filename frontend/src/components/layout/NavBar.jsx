import NofifIcon from "../../assets/icons/alert-outlined.svg?react";
import avatar from "../../assets/icons/profile.svg";
import { useLocation } from "react-router";

const pageTitles = {
  "/dashboard": "Tableau de bord",
  "/agents": "Gestion des agents",
  "/devices": "Gestion des appareils",
  "/checkpoints": "Points de contrôle",
  "/reports": "Rapports d'audit",
  "/settings": "Paramètres",
  "/signin": "Connexion",
};

const NavBar = ({
  username = "Jhon Doe",
  photoProfile = avatar,
  notificationCount = 0,
  onNotifClick,
  onProfileClick,
}) => {
  const location = useLocation();

  const title = pageTitles[location.pathname] ?? "Page introuvable";
  return (
    <header className="flex flex-row items-center justify-between w-full px-4 py-2 border-b bg-surface border-text-muted">
      <h1 className="font-sans text-2xl text-primary">{title}</h1>
      <div className="flex flex-row items-center justify-end gap-6">
        <button
          type="button"
          aria-label={`Notifications : ${notificationCount} non lues`}
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
        <div className="flex flex-row items-center justify-end gap-2">
          <h3
            className="font-sans text-sm text-text-secondary hover:cursor-pointer"
            onClick={onProfileClick}
          >
            {username}
          </h3>
          <img
            src={photoProfile}
            className="w-8 h-8 rounded-full bg-text-muted hover:cursor-pointer"
            alt="profile"
            onClick={onProfileClick}
          />
        </div>
      </div>
    </header>
  );
};

export default NavBar;
