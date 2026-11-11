import { useRef } from "react";
import CameraIcon from "../../../assets/icons/camera.svg?react";
import AvatarDefaulIcon from "../../../assets/icons/user.svg";
import useLanguage from "../../../context/useLanguage";

const AvatarUpload = ({ value = "", onChange, size = 240 }) => {
  const { t } = useLanguage();
  const inputRef = useRef(null);

  const handleImageChange = (event) => {
    const file = event.target.files?.[0];

    if (!file) return;

    if (
      !["image/jpeg", "image/png", "image/webp"].includes(file.type) ||
      !/\.(jpe?g|png|webp)$/i.test(file.name)
    ) {
      alert(t("Veuillez sélectionner une image JPEG, PNG ou WebP."));
      event.target.value = "";
      return;
    }

    if (file.size > 5 * 1024 * 1024) {
      alert(t("L'image ne doit pas dépasser 5 Mo."));
      event.target.value = "";
      return;
    }

    const imageUrl = URL.createObjectURL(file);
    onChange?.(file, imageUrl);
    event.target.value = "";
  };

  return (
    <div className="relative shrink-0" style={{ width: size, height: size }}>
      {/* Avatar */}
      <div className="w-full h-full overflow-hidden bg-gray-200 rounded-full">
        <img
          src={value || AvatarDefaulIcon}
          alt={value ? t("Photo de profil") : t("Avatar par défaut")}
          className={`h-full w-full ${
            value ? "object-cover" : "object-contain p-6"
          }`}
        />
      </div>

      {/* Sélecteur de fichier caché */}
      <input
        ref={inputRef}
        type="file"
        accept="image/*"
        className="hidden"
        onChange={handleImageChange}
      />

      {/* Bouton caméra */}
      <button
        type="button"
        onClick={() => inputRef.current?.click()}
        aria-label={t("Choisir une photo de profil")}
        title={t("Changer la photo")}
        className="absolute flex items-center justify-center w-10 h-10 text-text-primary transition bg-gray-200 rounded-lg shadow-md bottom-2 right-2 hover:bg-gray-100 focus:outline-none"
      >
        <CameraIcon className="h-7 w-7" />
      </button>
    </div>
  );
};

export default AvatarUpload;
