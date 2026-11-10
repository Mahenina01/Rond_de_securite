import { useState } from "react";
import { Outlet, useNavigate } from "react-router";
import Sidebar from "../components/layout/SideBar";
import Navbar from "../components/layout/NavBar";
import useLanguage from "../context/useLanguage";

const DashboardLayout = () => {
  const navigate = useNavigate();
  const { t } = useLanguage();
  const [isSidebarOpen, setIsSidebarOpen] = useState(false);

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
        <Sidebar onNavigate={() => setIsSidebarOpen(false)} />
      </aside>

      {/* Contenu principal */}
      <div className="flex flex-col flex-1 h-screen min-w-0 ml-0 overflow-hidden md:ml-64">
        {/* Navbar fixe en haut */}
        <header className="sticky top-0 z-30 shrink-0 bg-background">
          <Navbar
            notificationCount={28}
            onMenuClick={() => setIsSidebarOpen((open) => !open)}
            isMenuOpen={isSidebarOpen}
            onProfileClick={() => navigate("/settings")}
            onNotifClick={handleClickNotif}
          />
        </header>

        {/* Seul le contenu de la page défile */}
        <main className="flex-1 min-h-0 p-4 overflow-y-auto sm:p-6">
          <Outlet />
        </main>
      </div>
    </div>
  );
};

export default DashboardLayout;
