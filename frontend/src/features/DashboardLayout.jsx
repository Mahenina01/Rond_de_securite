import { useEffect, useMemo, useRef, useState } from "react";
import { Outlet, useNavigate } from "react-router";
import Sidebar from "../components/layout/SideBar";
import Navbar from "../components/layout/NavBar";
import useLanguage from "../context/useLanguage";
import logoService from "../service/logoService";
import authService from "../service/authService";
import notificationsService from "../service/notificationsService";
import { connectNotificationWebSocket } from "../service/notificationWebSocket";
import toast from "react-hot-toast";
import Input from "../components/ui/Input";
import IconNotif from "../assets/icons/alert-outlined.svg?react";

const mergeNotifications = (current, incoming) => {
  const notificationsById = new Map(
    current.map((notification) => [notification.id, notification]),
  );

  incoming.forEach((notification) => {
    notificationsById.set(notification.id, {
      ...notificationsById.get(notification.id),
      ...notification,
    });
  });

  return Array.from(notificationsById.values()).sort(
    (first, second) =>
      new Date(second.timestamp ?? 0).getTime() -
      new Date(first.timestamp ?? 0).getTime(),
  );
};

const DashboardLayout = () => {
  const navigate = useNavigate();
  const { t } = useLanguage();
  const [isSidebarOpen, setIsSidebarOpen] = useState(false);
  const [isNotificationPanelOpen, setIsNotificationPanelOpen] = useState(false);
  const [logoUrl, setLogoUrl] = useState(null);
  const [hasLogo, setHasLogo] = useState(false);
  const [profilePhotoUrl, setProfilePhotoUrl] = useState(null);
  const [notifications, setNotifications] = useState([]);
  const [notificationStatusFilter, setNotificationStatusFilter] =
    useState("ALL");
  const [notificationSearch, setNotificationSearch] = useState("");
  const [markingNotificationIds, setMarkingNotificationIds] = useState(
    () => new Set(),
  );
  const seenNotificationIds = useRef(new Set());

  const unreadNotificationCount = notifications.filter(
    (notification) => !notification.read,
  ).length;

  const filteredNotifications = useMemo(() => {
    const query = notificationSearch.trim().toLowerCase();

    return notifications.filter((notification) => {
      const matchesStatus =
        notificationStatusFilter === "ALL" ||
        (notificationStatusFilter === "UNREAD" && !notification.read) ||
        (notificationStatusFilter === "READ" && notification.read);

      const matchesSearch =
        !query ||
        [notification.title, notification.description].some((value) =>
          String(value ?? "")
            .toLowerCase()
            .includes(query),
        );

      return matchesStatus && matchesSearch;
    });
  }, [notifications, notificationSearch, notificationStatusFilter]);

  const handleNotificationClick = async (notification) => {
    if (notification.read || markingNotificationIds.has(notification.id)) {
      return;
    }

    setMarkingNotificationIds((current) =>
      new Set(current).add(notification.id),
    );

    try {
      const updatedNotification = await notificationsService.updateReadStatus(
        notification.id,
        true,
      );
      setNotifications((current) =>
        mergeNotifications(current, [
          updatedNotification?.data ?? updatedNotification,
        ]),
      );
    } catch (error) {
      toast.error(
        error.response?.data?.message ??
          t("Impossible de marquer la notification comme lue."),
      );
    } finally {
      setMarkingNotificationIds((current) => {
        const next = new Set(current);
        next.delete(notification.id);
        return next;
      });
    }
  };

  useEffect(() => {
    let cancelled = false;

    const loadLogo = async () => {
      try {
        const logo = await logoService.getCurrent();
        const imageUrl = await logoService.getImageUrl(logo?.photo);
        if (cancelled) {
          if (imageUrl) URL.revokeObjectURL(imageUrl);
          return;
        }
        setLogoUrl(imageUrl);
        setHasLogo(Boolean(logo?.photo));
      } catch (error) {
        if (cancelled) return;
        if (error.response?.status !== 404) {
          toast.error(
            error.response?.data?.message ??
              t("Impossible de charger le logo."),
          );
        }
      }
    };

    void loadLogo();

    return () => {
      cancelled = true;
    };
  }, [t]);

  useEffect(() => {
    let cancelled = false;

    const unsubscribe = connectNotificationWebSocket({
      onNotificationReceived: (notification) => {
        const alreadySeen = seenNotificationIds.current.has(notification.id);
        seenNotificationIds.current.add(notification.id);
        setNotifications((current) =>
          mergeNotifications(current, [notification]),
        );

        if (alreadySeen) return;

        const message = `${notification.title}: ${notification.description}`;
        if (notification.type === "NOT_VALIDATED") {
          toast.error(message);
        } else {
          toast.success(message);
        }
      },
      onError: (error) => {
        console.error("Erreur du WebSocket des notifications :", error);
      },
    });

    notificationsService
      .getAll()
      .then((response) => {
        if (cancelled) return;
        if (!Array.isArray(response)) {
          throw new Error(
            t("Le serveur n'a pas renvoyé une liste valide de notifications."),
          );
        }
        response.forEach((notification) =>
          seenNotificationIds.current.add(notification.id),
        );
        setNotifications((current) => mergeNotifications(current, response));
      })
      .catch((error) => {
        if (!cancelled) {
          toast.error(
            error.response?.data?.message ??
              t("Impossible de charger les notifications."),
          );
        }
      });

    return () => {
      cancelled = true;
      unsubscribe();
    };
  }, [t]);

  useEffect(() => {
    let cancelled = false;

    const loadProfilePhoto = async () => {
      try {
        const profile = await authService.getProfile();
        const photoUrl = await authService.getProfilePhotoPreview(
          profile.photo,
        );
        if (cancelled) {
          URL.revokeObjectURL(photoUrl);
          return;
        }
        setProfilePhotoUrl(photoUrl);
      } catch (error) {
        if (!cancelled) {
          toast.error(
            error.response?.data?.message ??
              t("Impossible de charger la photo de profil."),
          );
        }
      }
    };

    void loadProfilePhoto();
    return () => {
      cancelled = true;
    };
  }, [t]);

  useEffect(
    () => () => {
      if (logoUrl?.startsWith("blob:")) URL.revokeObjectURL(logoUrl);
    },
    [logoUrl],
  );

  useEffect(
    () => () => {
      if (profilePhotoUrl?.startsWith("blob:")) {
        URL.revokeObjectURL(profilePhotoUrl);
      }
    },
    [profilePhotoUrl],
  );

  const handleLogoUpdated = async (photoPath) => {
    const imageUrl = await logoService.getImageUrl(photoPath);
    setLogoUrl(imageUrl);
    setHasLogo(true);
  };

  const handleProfilePhotoUpdated = async () => {
    const profile = await authService.getProfile();
    setProfilePhotoUrl(await authService.getProfilePhotoPreview(profile.photo));
  };

  return (
    <div className="flex h-screen overflow-hidden bg-background">
      {isSidebarOpen && (
        <button
          type="button"
          aria-label={t("Fermer le menu")}
          onClick={() => setIsSidebarOpen(false)}
          className="fixed inset-0 z-40 bg-black/50 md:hidden"
        />
      )}

      <aside
        className={`fixed inset-y-0 left-0 z-50 h-screen transition-transform duration-300 ease-in-out md:translate-x-0 ${
          isSidebarOpen ? "translate-x-0" : "-translate-x-full"
        }`}
      >
        <Sidebar logoUrl={logoUrl} onNavigate={() => setIsSidebarOpen(false)} />
      </aside>

      {/* Contenu principal */}
      <div className="flex flex-col flex-1 h-screen min-w-0 ml-0 overflow-hidden md:ml-64">
        {/* Navbar fixe en haut */}
        <header className="sticky top-0 z-30 shrink-0 bg-background">
          <Navbar
            photoProfile={profilePhotoUrl || undefined}
            notificationCount={unreadNotificationCount}
            onMenuClick={() => setIsSidebarOpen((open) => !open)}
            isMenuOpen={isSidebarOpen}
            isNotificationPanelOpen={isNotificationPanelOpen}
            onProfileClick={() => navigate("/settings")}
            onNotifClick={() => setIsNotificationPanelOpen((isOpen) => !isOpen)}
          />
        </header>

        {/* Seul le contenu de la page défile */}
        <main className="flex-1 min-h-0 p-4 overflow-y-auto sm:p-6">
          <Outlet
            context={{
              hasLogo,
              logoUrl,
              onLogoUpdated: handleLogoUpdated,
              onProfilePhotoUpdated: handleProfilePhotoUpdated,
            }}
          />
        </main>
      </div>

      {isNotificationPanelOpen && (
        <aside
          aria-label={t("Notifications")}
          className="fixed right-3 top-16 z-[60] flex max-h-[calc(100vh-5rem)] w-[min(24rem,calc(100vw-1.5rem))] flex-col overflow-hidden rounded-xl bg-surface shadow-2xl"
        >
          <div className="flex items-center justify-between px-4 py-3">
            <div className="flex items-center gap-2">
              <IconNotif className="w-6 h-6 font-semibold text-text-primary" />
              <h2 className="font-semibold text-text-primary">
                {t("Notifications")}
              </h2>
            </div>
            <button
              type="button"
              onClick={() => setIsNotificationPanelOpen(false)}
              aria-label={t("Fermer les notifications")}
              className="p-2 rounded-md text-text-secondary hover:bg-gray-100"
            >
              ✕
            </button>
          </div>

          <div className="flex flex-col gap-3 p-4">
            <Input
              variant="search"
              value={notificationSearch}
              onChange={(event) => setNotificationSearch(event.target.value)}
              placeholder={t("Rechercher une notification...")}
              aria-label={t("Rechercher une notification...")}
              className="w-full px-3 py-2 text-sm border rounded-lg outline-none border-text-muted bg-surface text-text-primary placeholder:text-text-muted focus:border-primary focus:ring-2 focus:ring-blue-100"
            />
            <div className="flex flex-col gap-1 text-xs text-text-secondary">
              <div role="group" aria-label={t("État")} className="flex gap-1">
                {[
                  { value: "ALL", label: t("Tous") },
                  { value: "UNREAD", label: t("Non lu") },
                  { value: "READ", label: t("Lu") },
                ].map((option) => (
                  <button
                    key={option.value}
                    type="button"
                    aria-pressed={notificationStatusFilter === option.value}
                    onClick={() => setNotificationStatusFilter(option.value)}
                    className={`flex-1 border-b-2 px-2 py-2 text-sm font-medium transition-colors ${
                      notificationStatusFilter === option.value
                        ? "border-primary text-primary"
                        : "border-transparent text-text-secondary hover:bg-gray-50 hover:text-text-primary"
                    }`}
                  >
                    {option.label}
                  </button>
                ))}
              </div>
            </div>
          </div>

          <div className="flex-1 min-h-0 overflow-y-auto">
            {filteredNotifications.length ? (
              filteredNotifications.map((notification) => (
                <article
                  key={notification.id}
                  role="button"
                  tabIndex={notification.read ? -1 : 0}
                  onClick={() => void handleNotificationClick(notification)}
                  onKeyDown={(event) => {
                    if (event.key === "Enter" || event.key === " ") {
                      event.preventDefault();
                      void handleNotificationClick(notification);
                    }
                  }}
                  aria-disabled={
                    notification.read ||
                    markingNotificationIds.has(notification.id)
                  }
                  aria-label={`${notification.title || t("Aucun message")}. ${
                    notification.read ? t("Lu") : t("Marquer comme lu")
                  }`}
                  className={`w-full cursor-pointer border-b border-gray-100 px-4 py-3 text-left last:border-b-0 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-primary dark:border-gray-700 ${
                    notification.read ||
                    markingNotificationIds.has(notification.id)
                      ? "cursor-default"
                      : "hover:brightness-95"
                  } ${
                    notification.type === "VALIDATED"
                      ? " bg-emerald-50 dark:border-l-emerald-400 dark:bg-emerald-950/40"
                      : notification.type === "NOT_VALIDATED"
                        ? " bg-rose-50 dark:border-l-rose-400 dark:bg-rose-950/40"
                        : " bg-surface dark:border-l-gray-600"
                  }`}
                >
                  <h3 className="text-sm font-semibold text-text-primary">
                    {notification.title || t("Aucun message")}
                  </h3>
                  <p className="mt-1 text-sm break-words whitespace-pre-wrap text-text-secondary">
                    {notification.description}
                  </p>
                  <div className="flex items-center justify-between gap-2 mt-2 text-xs text-text-muted">
                    <svg
                      aria-hidden="true"
                      viewBox="0 0 24 24"
                      fill="none"
                      stroke="currentColor"
                      strokeWidth="2"
                      strokeLinecap="round"
                      strokeLinejoin="round"
                      className={`h-5 w-5 ${
                        notification.read
                          ? "text-green-600 dark:text-green-400"
                          : "text-gray-400 dark:text-gray-500"
                      }`}
                    >
                      <path d="m1 12 4 4L15 6" />
                      <path d="m9 12 4 4L23 6" />
                    </svg>
                    <time>
                      {notification.timestamp
                        ? new Date(notification.timestamp).toLocaleString()
                        : ""}
                    </time>
                  </div>
                </article>
              ))
            ) : (
              <p className="px-4 py-8 text-sm text-center text-text-secondary">
                {t("Aucune notification trouvée.")}
              </p>
            )}
          </div>
        </aside>
      )}
    </div>
  );
};

export default DashboardLayout;
