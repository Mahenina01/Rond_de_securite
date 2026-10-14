import checkIcon from "../assets/icons/verify.svg?react";
import airdropIcon from "../assets/icons/airdrop.svg?react";
import trendupIcon from "../assets/icons/trend-up.svg?react";

const cardTextVariant = {
  ronde_confirm: {
    icon: checkIcon,
    text: "Rondes confirmées aujourd'hui",
    color: "text-success",
  },

  device_actif_count: {
    icon: airdropIcon,
    text: "Dispositifs actifs",
    color: "text-primary",
  },

  succes_percent: {
    icon: trendupIcon,
    text: "Taux de succès d'envoi",
    color: "text-blue-500",
  },
};

export default cardTextVariant;
