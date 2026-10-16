import UserIcon from "../../assets/icons/user-round-plus.svg?react";

const Modal = ({
  isOpen,
  onClose,
  title,
  children,
  footer,
  IconTitle = { UserIcon },
}) => {
  if (!isOpen) return null;

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/50"
      onClick={onClose}
    >
      <div
        className="w-full max-w-lg p-6 bg-white shadow-xl rounded-xl"
        onClick={(event) => event.stopPropagation()}
        role="dialog"
        aria-modal="true"
        aria-labelledby="modal-title"
      >
        {/* En-tête */}
        <div className="flex items-center justify-between mb-5 ">
          <div className="flex flex-row items-center gap-3">
            <IconTitle className="w-8 h-8 text-text-primary" />
            <h2
              id="modal-title"
              className="text-xl font-semibold text-gray-900"
            >
              {title}
            </h2>
          </div>

          <button
            type="button"
            onClick={onClose}
            className="p-2 text-gray-500 rounded-md hover:bg-gray-100"
            aria-label="Fermer la fenêtre"
          >
            ✕
          </button>
        </div>

        {/* Contenu */}
        <div className="text-gray-600">{children}</div>

        {/* Pied de dialogue */}
        {footer && <div className="flex justify-end gap-3 mt-6">{footer}</div>}
      </div>
    </div>
  );
};

export default Modal;
