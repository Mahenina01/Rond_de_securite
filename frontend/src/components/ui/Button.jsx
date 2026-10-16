const baseStyles = [
  "inline-flex",
  "items-center",
  "justify-center",
  "gap-[10px]",
  "rounded-lg",
  "px-4",
  "py-3",
  "text-sm",
  "font-semibold",
  "transition-colors",
  "duration-200",
  "focus-visible:outline-none",
  "focus-visible:ring-2",
  "focus-visible:ring-primary",
  "focus-visible:ring-offset-2",
].join(" ");

const variants = {
  primary: [
    "bg-primary",
    "text-white",
    "border-2",
    "border-primary",
    "hover:bg-primary-hover",
    "hover:border-primary-hover",
    "active:bg-primary-hover",
  ].join(" "),

  outline: [
    "bg-transparent",
    "text-text-secondary",
    "border",
    "border-gray-300",
    "hover:bg-gray-100",
  ].join(" "),
  disabled: [
    "cursor-not-allowed",
    "bg-primary-light",
    "text-white",
    "border-2",
    "border-primary-light",
  ].join(" "),
  waiting: ["cursor-wait", "bg-primary-waiting", "text-white", "border-2"].join(
    " ",
  ),
};

const stateStyles = {
  disabled: "cursor-not-allowed bg-primary-light text-white",

  outline: [
    "cursor-not-allowed",
    "bg-transparent",
    "text-primary-outline",
    "border-2",
    "border-primary-outline",
  ].join(" "),

  waiting: "cursor-wait bg-primary-waiting text-white border-0",
};

export default function Button({
  children,
  variant = "primary",
  state = "default",
  disabled = false,
  type = "button",
  className = "",
  onClick,
  ...props
}) {
  const isDisabled =
    disabled ||
    state === "disabled" ||
    state === "outline" ||
    state === "waiting";

  let stateClass = variants[variant] || variants.primary;

  if (state === "disabled" || disabled) {
    stateClass = stateStyles.disabled;
  } else if (state === "outline") {
    stateClass = stateStyles.outline;
  } else if (state === "waiting") {
    stateClass = stateStyles.waiting;
  }

  return (
    <button
      type={type}
      disabled={isDisabled}
      onClick={onClick}
      className={`${baseStyles} ${stateClass} ${className}`}
      {...props}
    >
      {children}
    </button>
  );
}
