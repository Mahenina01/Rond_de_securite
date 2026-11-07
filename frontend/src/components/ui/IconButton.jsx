import textVariant from "../../constants/textSideBar";
import ExportIcon from "../../assets/icons/exporter.svg?react";

const IconButton = ({
  variant = "dashboard",
  active = false,
  onClick,
  isButton = false,
  iconSVG = ExportIcon,
  text = "button",
}) => {
  const config = textVariant[variant];

  if (!config) return null;

  const SelectedIcon = isButton ? iconSVG : config.icon;

  const className = isButton
    ? "flex items-center gap-3 rounded-lg bg-primary px-4 py-3 text-surface transition hover:bg-primary/90"
    : `flex w-full items-center gap-3 rounded-lg px-4 py-3 transition ${
        active
          ? "border-l-2 border-primary bg-white/10 text-white"
          : "border-l-2 border-transparent text-white hover:bg-white/10"
      }`;

  return (
    <button type="button" onClick={onClick} className={className}>
      <SelectedIcon className="w-5 h-5 shrink-0" />

      <span className="text-sm font-medium">
        {isButton ? text : config.text}
      </span>
    </button>
  );
};

export default IconButton;
