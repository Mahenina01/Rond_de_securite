import { NavLink, useNavigate } from "react-router";
import IconButton from "../ui/IconButton";
import LogOutButton from "../ui/LogOutButton";
import logo from "../../assets/images/logo.jpg";
import authService from "../../service/authService";
import toast from "react-hot-toast";
import useLanguage from "../../context/useLanguage";

const navigationItems = [
  { variant: "dashboard", label: "Tableau de bord", to: "/dashboard" },
  { variant: "agents", label: "Agents", to: "/agents" },
  { variant: "device", label: "Appareils", to: "/devices" },
  { variant: "location", label: "Points de contrôle", to: "/locations" },
  { variant: "logs", label: "Rapports", to: "/reports" },
  { variant: "setting", label: "Paramètres", to: "/settings" },
];

const SideBar = ({ onNavigate, logoUrl }) => {
  const navigate = useNavigate();
  const { t } = useLanguage();
  const handleLogout = async () => {
    try {
      await authService.logout();
      toast.success(t("Déconnexion réussie."));
    } catch (error) {
      console.error("Erreur lors de la déconnexion :", error);

      toast.error(
        error.response?.data?.message || t("Erreur lors de la déconnexion."),
      );
    } finally {
      navigate("/signin", { replace: true });
    }
  };

  return (
    <aside className="flex flex-col w-64 h-full min-h-0 p-4 border-r shrink-0 border-text-muted bg-[#1A2B4C]">
      <nav
        className="flex flex-col flex-1 gap-2 overflow-y-auto"
        aria-label={t("Navigation principale")}
      >
        <div className="flex flex-row items-center gap-2 pb-4">
          <img src={logoUrl || logo} alt="logo" className="w-8 h-8" />
          <h1 className="font-['Salsa',_sans-serif] text-3xl text-white font-bold">
            Rond<span className="font-mono text-primary">ix</span>
          </h1>
        </div>
        {navigationItems.map((item) => (
          <NavLink
            key={item.to}
            to={item.to}
            className="rounded-lg"
            onClick={onNavigate}
          >
            {({ isActive }) => (
              <IconButton variant={item.variant} active={isActive} />
            )}
          </NavLink>
        ))}
      </nav>

      <LogOutButton onClick={handleLogout} />
    </aside>
  );
};

export default SideBar;
