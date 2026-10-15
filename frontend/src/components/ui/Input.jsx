import { useState } from "react";

import searchIcon from "../../assets/icons/search.svg";
import EyeIcon from "../../assets/icons/show-eye.svg?react";
import EyeOffIcon from "../../assets/icons/hide-eye.svg?react";

const inputVariants = {
  default: {
    type: "text",
    placeholder: "Saisissez votre texte",
  },
  email: {
    type: "email",
    placeholder: "Entrez votre adresse e-mail",
  },
  search: {
    type: "search",
    placeholder: "Rechercher...",
  },
  password: {
    type: "password",
    placeholder: "Entrez votre mot de passe",
  },
};

const Input = ({
  variant = "default",
  label,
  name,
  value,
  onChange,
  placeholder,
  required = false,
  className = "",
}) => {
  const [showPassword, setShowPassword] = useState(false);

  const config = inputVariants[variant] ?? inputVariants.default;

  const isPassword = variant === "password";
  const isSearch = variant === "search";

  const inputType = isPassword && showPassword ? "text" : config.type;

  return (
    <div className="flex flex-col w-full gap-2">
      {label && (
        <label htmlFor={name} className="text-sm font-medium text-gray-700">
          {label}
          {required && <span className="ml-1 text-red-500">*</span>}
        </label>
      )}

      <div className="relative flex items-center">
        {isSearch && (
          <img
            src={searchIcon}
            alt=""
            aria-hidden="true"
            className="absolute w-5 h-5 pointer-events-none left-3"
          />
        )}

        <input
          id={name}
          name={name}
          type={inputType}
          placeholder={placeholder ?? config.placeholder}
          value={value}
          onChange={onChange}
          required={required}
          className={`
            w-full rounded-lg
            border border-text-muted
            bg-surface px-4 py-3 text-sm
            outline-none transition
            placeholder:text-text-muted
            focus:border-primary focus:ring-2 focus:ring-blue-100
            ${isSearch ? "pl-10" : ""}
            ${isPassword ? "pr-12" : ""}
            ${className}
          `}
        />

        {isPassword && (
          <button
            type="button"
            onClick={() => setShowPassword((previous) => !previous)}
            aria-label={
              showPassword
                ? "Masquer le mot de passe"
                : "Afficher le mot de passe"
            }
            aria-pressed={showPassword}
            className="absolute flex items-center justify-center p-1 rounded right-3"
          >
            {showPassword ? (
              <EyeOffIcon className="w-7 h-7 text-text-muted" />
            ) : (
              <EyeIcon className="w-7 h-7 text-text-muted" />
            )}
          </button>
        )}
      </div>
    </div>
  );
};

export default Input;
