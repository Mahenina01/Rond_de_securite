import { NavLink, useNavigate } from "react-router";
import IconButton from "../ui/IconButton";
import LogOutButton from "../ui/LogOutButton";
import logo from "../../assets/images/logo.jpg";
import authService from "../../service/authService";
import toast from "react-hot-toast";

const navigationItems = [
  { variant: "dashboard", label: "Tableau de bord", to: "/dashboard" },
  { variant: "agents", label: "Agents", to: "/agents" },
  { variant: "device", label: "Appareils", to: "/devices" },
  { variant: "location", label: "Points de contrôle", to: "/locations" },
  { variant: "logs", label: "Rapports", to: "/reports" },
  { variant: "setting", label: "Paramètres", to: "/settings" },
];

const SideBar = () => {
  const navigate = useNavigate();
  const handleLogout = async () => {
    try {
      await authService.logout();
      toast.success("Déconnexion réussie.");
    } catch (error) {
      console.error("Erreur lors de la déconnexion :", error);

      toast.error(
        error.response?.data?.message || "Erreur lors de la déconnexion.",
      );
    } finally {
      navigate("/signin", { replace: true });
    }
  };

  return (
    <aside className="flex flex-col w-64 min-h-screen p-4 border-r shrink-0 border-text-muted bg-surface">
      <nav
        className="flex flex-col flex-1 gap-2"
        aria-label="Navigation principale"
      >
        <div className="flex flex-row items-center gap-2 pb-4">
          <img src={logo} alt="logo" className="w-8 h-8 rounded-full" />
          <h1 className="font-['Salsa',_sans-serif] text-2xl text-primary">
            Ronde Security
          </h1>
        </div>
        {navigationItems.map((item) => (
          <NavLink
            key={item.to}
            to={item.to}
            className={({ isActive }) =>
              `rounded-lg ${isActive ? "bg-primary/10 text-primary" : ""}`
            }
          >
            <IconButton variant={item.variant} />
          </NavLink>
        ))}
      </nav>

      <LogOutButton onClick={handleLogout} />
    </aside>
  );
};

export default SideBar;
