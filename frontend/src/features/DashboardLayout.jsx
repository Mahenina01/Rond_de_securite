import { useEffect, useState } from "react";
import { Outlet, useNavigate } from "react-router";
import Sidebar from "../components/layout/SideBar";
import Navbar from "../components/layout/NavBar";
import useLanguage from "../context/useLanguage";
import logoService from "../service/logoService";
import authService from "../service/authService";
import toast from "react-hot-toast";

const DashboardLayout = () => {
  const navigate = useNavigate();
  const { t } = useLanguage();
  const [isSidebarOpen, setIsSidebarOpen] = useState(false);
  const [logoUrl, setLogoUrl] = useState(null);
  const [hasLogo, setHasLogo] = useState(false);
  const [profilePhotoUrl, setProfilePhotoUrl] = useState(null);

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
            error.response?.data?.message ?? t("Impossible de charger le logo."),
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

    const loadProfilePhoto = async () => {
      try {
        const profile = await authService.getProfile();
        const photoUrl = await authService.getProfilePhotoPreview(profile.photo);
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
    setProfilePhotoUrl(
      await authService.getProfilePhotoPreview(profile.photo),
    );
  };

  const handleClickNotif = () => {
    alert(t("Notification cliquée"));
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
        <Sidebar
          logoUrl={logoUrl}
          onNavigate={() => setIsSidebarOpen(false)}
        />
      </aside>

      {/* Contenu principal */}
      <div className="flex flex-col flex-1 h-screen min-w-0 ml-0 overflow-hidden md:ml-64">
        {/* Navbar fixe en haut */}
        <header className="sticky top-0 z-30 shrink-0 bg-background">
          <Navbar
            photoProfile={profilePhotoUrl || undefined}
            notificationCount={28}
            onMenuClick={() => setIsSidebarOpen((open) => !open)}
            isMenuOpen={isSidebarOpen}
            onProfileClick={() => navigate("/settings")}
            onNotifClick={handleClickNotif}
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
    </div>
  );
};

export default DashboardLayout;
