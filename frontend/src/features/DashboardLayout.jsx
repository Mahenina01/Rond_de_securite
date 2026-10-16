import { Outlet, useNavigate } from "react-router";
import Sidebar from "../components/layout/SideBar";
import Navbar from "../components/layout/NavBar";

const DashboardLayout = () => {
  const navigate = useNavigate();

  const handleClickNotif = () => {
    alert("Notification cliquée");
  };

  return (
    <div className="flex h-screen overflow-hidden bg-background">
      {/* Sidebar fixe */}
      <aside className="fixed inset-y-0 left-0 z-40 h-screen">
        <Sidebar />
      </aside>

      {/* Contenu principal */}
      <div className="flex flex-col flex-1 h-screen min-w-0 ml-0 overflow-hidden md:ml-64">
        {/* Navbar fixe en haut */}
        <header className="sticky top-0 z-30 shrink-0 bg-background">
          <Navbar
            notificationCount={28}
            onProfileClick={() => navigate("/settings")}
            onNotifClick={handleClickNotif}
          />
        </header>

        {/* Seul le contenu de la page défile */}
        <main className="flex-1 min-h-0 p-6 overflow-y-auto">
          <Outlet />
        </main>
      </div>
    </div>
  );
};

export default DashboardLayout;
