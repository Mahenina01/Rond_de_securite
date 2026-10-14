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
    ? "flex items-center gap-3 rounded-lg bg-primary px-4 py-3 text-surface transition hover:bg-primary-hover"
    : `flex w-full items-center gap-3 rounded-lg px-4 py-3 transition ${
        active
          ? "bg-blue-50 text-blue-600"
          : "text-text-secondary hover:bg-gray-100"
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
