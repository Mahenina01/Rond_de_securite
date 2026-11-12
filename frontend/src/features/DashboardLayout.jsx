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

const getTime = (notification) =>
  new Date(notification.timestamp ?? 0).getTime();

const formatRelativeTime = (timestamp) => {
  const date = new Date(timestamp);
  if (Number.isNaN(date.getTime())) return "";

  const elapsedSeconds = Math.round((date.getTime() - Date.now()) / 1000);
  const elapsedMinutes = Math.round(elapsedSeconds / 60);
  const elapsedHours = Math.round(elapsedMinutes / 60);
  const elapsedDays = Math.round(elapsedHours / 24);
  const formatter = new Intl.RelativeTimeFormat(undefined, { numeric: "auto" });

  if (Math.abs(elapsedSeconds) < 60) {
    return formatter.format(elapsedSeconds, "second");
  }
  if (Math.abs(elapsedMinutes) < 60) {
    return formatter.format(elapsedMinutes, "minute");
  }
  if (Math.abs(elapsedHours) < 24) {
    return formatter.format(elapsedHours, "hour");
  }
  return formatter.format(elapsedDays, "day");
};

const NotificationTypeIcon = ({ type }) => {
  const isValidated = type === "VALIDATED";

  return (
    <svg
      aria-hidden="true"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth="1.8"
      strokeLinecap="round"
      strokeLinejoin="round"
      className={`h-7 w-7 shrink-0 ${
        isValidated ? "text-emerald-600" : "text-red-600"
      }`}
    >
      <>
        <path d="M18 9a6 6 0 0 0-12 0c0 7-3 7-3 9h18c0-2-3-2-3-9" />
        <path d="M10 21h4" />
      </>
    </svg>
  );
};

const mergeNotifications = (current, incoming) => {
  const byId = new Map(current.map((n) => [n.id, n]));
  incoming.forEach((n) => byId.set(n.id, { ...byId.get(n.id), ...n }));
  return [...byId.values()].sort((a, b) => getTime(b) - getTime(a));
};

const revokeBlobUrl = (url) => {
  if (url?.startsWith("blob:")) URL.revokeObjectURL(url);
};

const showError = (error, fallback) =>
  toast.error(error.response?.data?.message ?? fallback);

const DashboardLayout = () => {
  const navigate = useNavigate();
  const { t } = useLanguage();
  const [isSidebarOpen, setIsSidebarOpen] = useState(false);
  const [isNotificationPanelOpen, setIsNotificationPanelOpen] = useState(false);
  const [logoUrl, setLogoUrl] = useState(null);
  const [hasLogo, setHasLogo] = useState(false);
  const [profilePhotoUrl, setProfilePhotoUrl] = useState(null);
  const [notifications, setNotifications] = useState([]);
  const [statusFilter, setStatusFilter] = useState("ALL");
  const [search, setSearch] = useState("");
  const [markingIds, setMarkingIds] = useState(() => new Set());
  const seenIds = useRef(new Set());

  const unreadCount = notifications.filter((n) => !n.read).length;

  const filteredNotifications = useMemo(() => {
    const query = search.trim().toLowerCase();

    return notifications.filter((notification) => {
      const matchesStatus =
        statusFilter === "ALL" ||
        (statusFilter === "READ") === Boolean(notification.read);

      const matchesSearch =
        !query ||
        [notification.title, notification.description].some((value) =>
          String(value ?? "")
            .toLowerCase()
            .includes(query),
        );

      return matchesStatus && matchesSearch;
    });
  }, [notifications, search, statusFilter]);

  const handleNotificationClick = async (notification) => {
    if (notification.read || markingIds.has(notification.id)) return;

    setMarkingIds((current) => new Set(current).add(notification.id));

    try {
      const response = await notificationsService.updateReadStatus(
        notification.id,
        true,
      );
      setNotifications((current) =>
        mergeNotifications(current, [response?.data ?? response]),
      );
    } catch (error) {
      showError(error, t("Impossible de marquer la notification comme lue."));
    } finally {
      setMarkingIds((current) => {
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
          revokeBlobUrl(imageUrl);
          return;
        }
        setLogoUrl(imageUrl);
        setHasLogo(Boolean(logo?.photo));
      } catch (error) {
        if (!cancelled && error.response?.status !== 404) {
          showError(error, t("Impossible de charger le logo."));
        }
      }
    };

    loadLogo();
    return () => {
      cancelled = true;
    };
  }, [t]);

  useEffect(() => {
    let cancelled = false;

    const unsubscribe = connectNotificationWebSocket({
      onNotificationReceived: (notification) => {
        const isNew = !seenIds.current.has(notification.id);
        seenIds.current.add(notification.id);
        setNotifications((current) =>
          mergeNotifications(current, [notification]),
        );

        if (!isNew) return;

        const show =
          notification.type === "NOT_VALIDATED" ? toast.error : toast.success;
        show(`${notification.title}: ${notification.description}`);
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
        response.forEach((n) => seenIds.current.add(n.id));
        setNotifications((current) => mergeNotifications(current, response));
      })
      .catch((error) => {
        if (!cancelled) {
          showError(error, t("Impossible de charger les notifications."));
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
          revokeBlobUrl(photoUrl);
          return;
        }
        setProfilePhotoUrl(photoUrl);
      } catch (error) {
        if (!cancelled) {
          showError(error, t("Impossible de charger la photo de profil."));
        }
      }
    };

    loadProfilePhoto();
    return () => {
      cancelled = true;
    };
  }, [t]);

  useEffect(() => () => revokeBlobUrl(logoUrl), [logoUrl]);
  useEffect(() => () => revokeBlobUrl(profilePhotoUrl), [profilePhotoUrl]);

  const handleLogoUpdated = async (photoPath) => {
    setLogoUrl(await logoService.getImageUrl(photoPath));
    setHasLogo(true);
  };

  const handleProfilePhotoUpdated = async () => {
    const profile = await authService.getProfile();
    setProfilePhotoUrl(await authService.getProfilePhotoPreview(profile.photo));
  };

  const statusOptions = [
    { value: "ALL", label: t("Tous") },
    { value: "UNREAD", label: t("Non lu") },
    { value: "READ", label: t("Lu") },
  ];

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
        className={`fixed inset-y-0 left-0 z-50 transition-transform duration-300 ease-in-out md:translate-x-0 ${
          isSidebarOpen ? "translate-x-0" : "-translate-x-full"
        }`}
      >
        <Sidebar logoUrl={logoUrl} onNavigate={() => setIsSidebarOpen(false)} />
      </aside>

      <div className="flex flex-col flex-1 min-w-0 overflow-hidden md:ml-64">
        <header className="z-30 shrink-0 bg-background">
          <Navbar
            photoProfile={profilePhotoUrl || undefined}
            notificationCount={unreadCount}
            onMenuClick={() => setIsSidebarOpen((open) => !open)}
            isMenuOpen={isSidebarOpen}
            isNotificationPanelOpen={isNotificationPanelOpen}
            onProfileClick={() => navigate("/settings")}
            onNotifClick={() => setIsNotificationPanelOpen((open) => !open)}
          />
        </header>

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
              <IconNotif className="w-6 h-6 text-text-primary" />
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
              value={search}
              onChange={(event) => setSearch(event.target.value)}
              placeholder={t("Rechercher une notification...")}
              aria-label={t("Rechercher une notification...")}
              className="w-full px-3 py-2 text-sm border rounded-lg outline-none border-text-muted bg-surface text-text-primary placeholder:text-text-muted focus:border-primary focus:ring-2 focus:ring-blue-100"
            />
            <div role="group" aria-label={t("État")} className="flex gap-1">
              {statusOptions.map((option) => (
                <button
                  key={option.value}
                  type="button"
                  aria-pressed={statusFilter === option.value}
                  onClick={() => setStatusFilter(option.value)}
                  className={`flex-1 border-b-2 px-2 py-2 text-sm font-medium transition-colors ${
                    statusFilter === option.value
                      ? "border-primary text-primary"
                      : "border-transparent text-text-secondary hover:bg-gray-50 hover:text-text-primary"
                  }`}
                >
                  {option.label}
                </button>
              ))}
            </div>
          </div>

          <div className="flex-1 min-h-0 overflow-y-auto">
            {filteredNotifications.length ? (
              filteredNotifications.map((notification) => {
                const isLocked =
                  notification.read || markingIds.has(notification.id);
                const timestamp = notification.timestamp
                  ? new Date(notification.timestamp)
                  : null;

                return (
                  <article
                    key={notification.id}
                    role="button"
                    tabIndex={notification.read ? -1 : 0}
                    onClick={() => handleNotificationClick(notification)}
                    onKeyDown={(event) => {
                      if (event.key === "Enter" || event.key === " ") {
                        event.preventDefault();
                        handleNotificationClick(notification);
                      }
                    }}
                    aria-disabled={isLocked}
                    aria-label={`${notification.title || t("Aucun message")}. ${
                      notification.read ? t("Lu") : t("Marquer comme lu")
                    }`}
                    className={`flex w-full items-start gap-3 border-b border-gray-200 px-4 py-4 text-left last:border-b-0 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-primary dark:border-gray-700 ${
                      notification.read
                        ? "border-l-4 border-l-transparent bg-surface"
                        : "cursor-pointer border-l-4 border-l-transparent bg-blue-50 hover:bg-blue-100 dark:bg-blue-950/30 dark:hover:bg-blue-950/50"
                    } ${isLocked && !notification.read ? "cursor-wait" : ""}`}
                  >
                    <NotificationTypeIcon type={notification.type} />
                    <div className="flex-1 min-w-0">
                      <h3 className="text-sm font-semibold text-text-primary">
                        {notification.title || t("Aucun message")}
                      </h3>
                      <p className="mt-1 text-sm break-words whitespace-pre-wrap text-text-secondary">
                        {notification.description}
                      </p>
                      {timestamp && !Number.isNaN(timestamp.getTime()) && (
                        <time
                          className="block mt-2 text-xs text-text-muted"
                          dateTime={timestamp.toISOString()}
                        >
                          {formatRelativeTime(timestamp)}
                        </time>
                      )}
                    </div>
                  </article>
                );
              })
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
