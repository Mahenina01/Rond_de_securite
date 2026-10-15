import { useRef, useState } from "react";
import CameraIcon from "../../../assets/icons/camera.svg?react";
import AvatarDefaulIcon from "../../../assets/icons/user.svg";

const AvatarUpload = ({ initialImage = "", onImageChange, size = 240 }) => {
  const inputRef = useRef(null);
  const [preview, setPreview] = useState(initialImage);

  const handleImageChange = (event) => {
    const file = event.target.files?.[0];

    if (!file) return;

    // Vérifier le format de l'image
    if (!file.type.startsWith("image/")) {
      alert("Veuillez sélectionner une image valide.");
      event.target.value = "";
      return;
    }

    // Limiter la taille à 5 Mo
    if (file.size > 5 * 1024 * 1024) {
      alert("L'image ne doit pas dépasser 5 Mo.");
      event.target.value = "";
      return;
    }

    const imageUrl = URL.createObjectURL(file);

    setPreview((previous) => {
      if (previous?.startsWith("blob:")) {
        URL.revokeObjectURL(previous);
      }
      return imageUrl;
    });

    onImageChange?.(file);
    event.target.value = "";
  };

  return (
    <div className="relative shrink-0" style={{ width: size, height: size }}>
      {/* Avatar */}
      <div className="w-full h-full overflow-hidden bg-gray-200 rounded-full">
        <img
          src={preview || AvatarDefaulIcon}
          alt={preview ? "Photo de profil" : "Avatar par défaut"}
          className={`h-full w-full ${
            preview ? "object-cover" : "object-contain p-6"
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
        aria-label="Choisir une photo de profil"
        title="Changer la photo"
        className="absolute flex items-center justify-center w-10 h-10 text-black transition bg-gray-200 rounded-lg shadow-md bottom-2 right-2 hover:bg-gray-100 focus:outline-none"
      >
        <CameraIcon className="h-7 w-7" />
      </button>
    </div>
  );
};

export default AvatarUpload;
