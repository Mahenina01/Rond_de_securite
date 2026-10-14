import { NavLink } from "react-router";
import IconButton from "../ui/IconButton";
import LogOutButton from "../ui/LogOutButton";
import logo from "../../assets/images/logo.jpg";

const navigationItems = [
  { variant: "dashboard", label: "Tableau de bord", to: "/dashboard" },
  { variant: "agents", label: "Agents", to: "/agents" },
  { variant: "device", label: "Appareils", to: "/devices" },
  { variant: "location", label: "Points de contrôle", to: "/locations" },
  { variant: "logs", label: "Rapports", to: "/reports" },
  { variant: "setting", label: "Paramètres", to: "/settings" },
];

const SideBar = () => {
  const handleLogout = () => {
    // TODO: Ajouter ici la logique réelle de déconnexion.
    console.log("Déconnexion");
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
            <IconButton variant={item.variant} onClick={() => {}} />
          </NavLink>
        ))}
      </nav>

      <LogOutButton onClick={handleLogout} />
    </aside>
  );
};

export default SideBar;
