import DashboardIcon from "../assets/icons/dashboard-outlined.svg?react";
import AgentIcon from "../assets/icons/users-outlined.svg?react";
import DeviceIcon from "../assets/icons/device-out.svg?react";
import LocationIcon from "../assets/icons/signpost.svg?react";
import LogIcon from "../assets/icons/export.svg?react";
import SettingIcon from "../assets/icons/setting-outlined.svg?react";

const textVariant = {
  dashboard: {
    icon: DashboardIcon,
    text: "Tableau de bord",
  },
  agents: {
    icon: AgentIcon,
    text: "Agents",
  },
  device: {
    icon: DeviceIcon,
    text: "Appareils",
  },
  location: {
    icon: LocationIcon,
    text: "Emplacements",
  },
  logs: {
    icon: LogIcon,
    text: "Rapport d'audit",
  },
  setting: {
    icon: SettingIcon,
    text: "Paramètres",
  },
};

export default textVariant;
