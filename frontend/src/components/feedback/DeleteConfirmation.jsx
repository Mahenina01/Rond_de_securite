import Warning from "../../assets/icons/alert 1.svg?react";
const DeleteConfirmation = ({ onCancelClick, onDeleteClick }) => {
  return (
    <div className="w-[400px] flex flex-col items-start justify-center gap-2 p-6 rounded-[20px] bg-surface">
      <div className="flex flex-row items-center justify-start gap-2">
        <Warning className="w-10 h-10 text-danger" />
        <h1 className="text-[20px] font-sans text-text-primary font-bold">
          Confirmer la suppression
        </h1>
      </div>
      <p className="py-4 text-sm text-text-secondary">
        Êtes-vous sûr de vouloir supprimer cet élément ? Cette action est
        irréversible et les données supprimées ne pourront pas être récupérées.
      </p>
      <div className="flex flex-row items-end justify-end w-full gap-2">
        <button
          type="button"
          onclick={onCancelClick}
          className="p-2 text-sm border-2 rounded-[10px] hover:bg-text-muted border-text-secondary text-text-secondary"
        >
          Annuler
        </button>
        <button
          type="button"
          onclick={onDeleteClick}
          className="p-2 rounded-[10px] bg-danger text-surface hover:bg-red-700"
        >
          Supprimer
        </button>
      </div>
    </div>
  );
};

export default DeleteConfirmation;
