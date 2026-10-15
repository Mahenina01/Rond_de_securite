export default function DataTable({ columns, data, onEdit, onDelete, onCopy }) {
  return (
    // Conteneur principal avec défilement horizontal si l'écran est trop petit
    <div className="w-full overflow-x-auto">
      <table className="w-full text-left border-collapse">
        <thead>
          <tr className="text-xs font-semibold tracking-wider text-gray-500 uppercase border-b border-gray-200">
            {columns.map((col, index) => (
              <th
                key={index}
                className={`py-3 px-4 ${col.align === "right" ? "text-right" : ""}`}
              >
                {col.header}
              </th>
            ))}
          </tr>
        </thead>

        <tbody className="text-sm text-gray-800 divide-y divide-gray-100">
          {/* Vérification conditionnelle : Est-ce qu'il y a des données à afficher ? */}
          {data.length > 0 ? (
            // Si OUI : on parcourt chaque ligne (row) de données
            data.map((row, rowIndex) => (
              <tr
                key={row.id || rowIndex}
                className="transition-colors hover:bg-gray-50/50"
              >
                {/* Pour chaque ligne, on parcourt les colonnes pour afficher les cellules correspondantes */}
                {columns.map((col, colIndex) => (
                  <td
                    key={colIndex}
                    className={`py-4 px-4 ${col.align === "right" ? "text-right" : ""}`}
                  >
                    {col.render
                      ? col.render(row, { onEdit, onDelete, onCopy })
                      : row[col.accessor]}
                  </td>
                ))}
              </tr>
            ))
          ) : (
            // Si NON : on affiche une ligne unique indiquant qu'aucun résultat n'a été trouvé
            <tr>
              <td
                colSpan={columns.length}
                className="py-6 text-sm text-center text-gray-500"
              >
                Aucun résultat trouvé.
              </td>
            </tr>
          )}
        </tbody>
      </table>
    </div>
  );
}
