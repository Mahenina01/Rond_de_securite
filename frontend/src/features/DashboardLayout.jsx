import { Outlet, useNavigate } from "react-router";
import Sidebar from "../components/layout/SideBar";
import Navbar from "../components/layout/NavBar";

const DashboardLayout = () => {
  const navigate = useNavigate();

  const handleClickNotif = () => {
    alert("Notification cliqué"); //TODO : show nofication
  };
  return (
    <div className="flex min-h-screen bg-background">
      <Sidebar />

      <div className="flex flex-col flex-1 min-w-0">
        <Navbar
          notificationCount={28}
          onProfileClick={() => navigate("/settings")}
          onNotifClick={handleClickNotif}
        />

        <main className="flex-1 p-6">
          <Outlet />
        </main>
      </div>
    </div>
  );
};

export default DashboardLayout;
